type result = PASS | FAIL | XPASS | XFAIL | ERROR | UNSUPPORTED | UNRESOLVED
            | WARNING | UNTESTED | KFAIL | GDB_compile_failed | Misc_error

type line_type = Result of result * string
	       | Test_error of result * string
	       | Begin_exp of string
	       | Summary of string
	       | Unknown

let output_line ofh str =
  Printf.fprintf ofh "%s\n" str

let rec skip_header fh ofh =
  try
    let line = input_line fh in
    output_line ofh line;
    if String.length line >= 7 && (String.sub line 0 7) = "Running" then ()
    else skip_header fh ofh
  with End_of_file -> ()

exception Bad_line

let running_exp_file = Str.regexp "^Running \\([^ ]*\\)"

let summary_line = Str.regexp "^[ \t]*=== [^ ]+ Summary ==="

let classify line =
  if Str.string_match running_exp_file line 0 then
    Begin_exp (Str.matched_group 1 line)
  else if Str.string_match summary_line line 0 then
    Summary line
  else begin
    try
      let idx = String.index line ':' in
      let res = String.sub line 0 idx
      and test = String.sub line (idx + 2) ((String.length line) - (idx + 2)) in
      let restype = match res with
	"PASS" -> PASS
      | "FAIL" -> FAIL
      | "XPASS" -> XPASS
      | "XFAIL" -> XFAIL
      | "ERROR" -> ERROR
      | "UNSUPPORTED" -> UNSUPPORTED
      | "UNRESOLVED" -> UNRESOLVED
      | "WARNING" -> WARNING
      | "UNTESTED" -> UNTESTED
      | "KFAIL" -> KFAIL
      | _ -> raise Bad_line in
      Result (restype, test)
    with
      Not_found -> Unknown
    | Invalid_argument "String.sub" -> raise Bad_line
  end

let contains_regex regex str =
  try
    let _ = Str.search_forward regex str 0 in
    true
  with Not_found -> false

let expfile_strip expfile testsuite_dir =
  let path = FilePath.DefaultPath.filename_of_string expfile in
  let rec scan p =
    if FilePath.basename p = testsuite_dir then
      FilePath.make_relative p path
    else
      let updir = FilePath.dirname p in
      if FilePath.is_updir updir p then scan updir else expfile in
  scan path

let string_of_result = function
    PASS -> "PASS"
  | FAIL -> "FAIL"
  | XPASS -> "XPASS"
  | XFAIL -> "XFAIL"
  | ERROR -> "ERROR"
  | UNSUPPORTED -> "UNSUPPORTED"
  | UNRESOLVED -> "UNRESOLVED"
  | WARNING -> "WARNING"
  | UNTESTED -> "UNTESTED"
  | KFAIL -> "KFAIL"
  | GDB_compile_failed -> "GDB_compile_failed"
  | Misc_error -> "Misc_error"

exception Changed

let process_sum_file sumfile outfile xfails =
  let gdb_comp_failed = Str.regexp "\\(^gdb compile failed, \\)" in
  let line_number = ref 1
  and current_expfile = ref "(unknown exp file)"
  and fh = open_in sumfile
  and ofh = open_out outfile in
  let unrescount = ref 0
  and passcount = ref 0
  and failcount = ref 0
  and xfailcount = ref 0
  and xpasscount = ref 0
  and unsupcount = ref 0 in
  skip_header fh ofh;
  try
    while true do
      let line = input_line fh in
      let classified =
        try
          classify line
	with Bad_line ->
	  if Str.string_match gdb_comp_failed line 0 then
	    let testname = Str.string_after line (Str.match_end ()) in
	    Test_error (GDB_compile_failed, testname)
	  else
	    Test_error (Misc_error, line) in
	begin match classified with
          Unknown ->
	    output_line ofh line
	| Begin_exp e ->
	    let stripped = expfile_strip e "testsuite" in
	    current_expfile := stripped;
	    output_line ofh line
	| Result (res, test) ->
	    begin try
	      begin match res with
		PASS -> incr passcount
	      | XFAIL -> incr xfailcount
	      | XPASS -> incr xpasscount
	      | UNSUPPORTED -> incr unsupcount
	      | UNRESOLVED -> incr unrescount
	      | FAIL ->
		  begin try
		    match Hashtbl.find xfails test with
	              FAIL ->
			incr xfailcount;
			Printf.fprintf ofh "XFAIL: %s\n" test;
			raise Changed
		    | _ -> failwith "XFAIL file should contain FAIL lines only"
		  with Not_found ->
		    incr failcount
		  end
	      | _ -> ()
	      end;
	      output_line ofh line
	    with Changed -> ()
	    end
	| Test_error (res, test) ->
	    output_line ofh line
	| Summary line ->
	    output_line ofh line;
	    raise End_of_file
	end;
	incr line_number
    done;
  with End_of_file ->
    let print_summary ofh =
      Printf.fprintf ofh "\n# of expected passes\t\t%d\n" !passcount;
      Printf.fprintf ofh "# of unexpected failures\t%d\n" !failcount;
      Printf.fprintf ofh "# of unexpected successes\t%d\n" !xpasscount;
      Printf.fprintf ofh "# of expected failures\t\t%d\n" !xfailcount;
      Printf.fprintf ofh "# of unresolved testcases\t%d\n" !unrescount;
      Printf.fprintf ofh "# of unsupported testcases\t%d\n" !unsupcount in
    print_summary ofh;
    print_summary stdout;
    close_in fh;
    close_out ofh

let read_xfail_file xfailfile =
  let line_number = ref 1
  and fh = open_in xfailfile
  and ht = Hashtbl.create 30 in
  try
    while true do
      let line = input_line fh in
      let classified =
        try
	  classify line
	with Bad_line ->
	  failwith (Printf.sprintf "Bad line in xfail file (%d)"
				   !line_number) in
      begin match classified with
        Unknown | Begin_exp _ | Summary _ -> ()
      | Result (res, test)
      | Test_error (res, test) ->
          if not (Hashtbl.mem ht test) then
	    Hashtbl.add ht test res
      end;
      incr line_number
    done;
    failwith "unreachable"
  with End_of_file -> close_in fh; ht

let _ =
  let insum = ref ""
  and outsum = ref ""
  and xfailfile = ref "" in
  let args = [
    "-i", Arg.Set_string insum, "Input sum file";
    "-o", Arg.Set_string outsum, "Output sum file";
    "-x", Arg.Set_string xfailfile, "XFAILs file"
  ]
  and usage = "Usage: failproc -i <insum> -o <outsum> -x <xfails>" in
  Arg.parse args (fun _ -> ()) usage;
  if !insum = "" || !outsum = "" || !xfailfile = "" then
    failwith usage;
  let xfails = read_xfail_file !xfailfile in
  process_sum_file !insum !outsum xfails
