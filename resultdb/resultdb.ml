type result = PASS | FAIL | XPASS | XFAIL | ERROR | UNSUPPORTED | UNRESOLVED
            | WARNING | UNTESTED | KFAIL | GDB_compile_failed | Misc_error

type line_type = Result of result * string
	       | Test_error of result * string
	       | Begin_exp of string
	       | Unknown

let rec skip_header fh =
  try
    let line = input_line fh in
    if String.length line >= 7 && (String.sub line 0 7) = "Running" then ()
    else skip_header fh
  with End_of_file -> ()

exception Bad_line

let running_exp_file = Str.regexp "^Running \\([^ ]*\\)"

let classify line =
  if Str.string_match running_exp_file line 0 then begin
    Begin_exp (Str.matched_group 1 line)
  end else begin
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

exception Too_many

let one_result = function
    [] -> raise Not_found
  | [x] -> x
  | _ -> raise Too_many

let expfile_id dbh expfile =
  try
    one_result (PGSQL (dbh) "select exp_id from expfiles where name=$expfile")
  with Not_found ->
    one_result (PGSQL (dbh) "insert into expfiles (name) values ($expfile) \
			     returning exp_id")

let component_id dbh component =
  try
    one_result (PGSQL (dbh) "select comp_id from components \
			     where component=$component")
  with Not_found ->
    one_result (PGSQL (dbh) "insert into components (component) \
			     values ($component) returning comp_id")

let test_id dbh comp_id exp_id testname =
  try
    one_result (PGSQL (dbh) "select test_id from tests where \
			     comp_id=$comp_id and exp_id=$exp_id \
			     and name=$testname")
  with Not_found ->
    one_result (PGSQL (dbh) "insert into tests (comp_id, exp_id, name) \
			     values ($comp_id, $exp_id, $testname) \
			     returning test_id")

let target_id dbh target =
  try
    one_result (PGSQL (dbh) "select target_id from targets \
			     where target=$target")
  with Not_found ->
    one_result (PGSQL (dbh) "insert into targets (target) values ($target) \
			     returning target_id")

let multilib_id dbh mlib =
  try
    one_result (PGSQL (dbh) "select mlib_id from multilibs \
			     where multilib=$mlib")
  with Not_found ->
    one_result (PGSQL (dbh) "insert into multilibs (multilib) values ($mlib) \
			     returning mlib_id")

let config_id dbh config =
  try
    one_result (PGSQL (dbh) "select cfg_id from configs \
			     where config=$config")
  with Not_found ->
    one_result (PGSQL (dbh) "insert into configs (config) values ($config) \
			     returning cfg_id")

let create_bundle dbh target_id cfg_id datestamp =
  one_result (PGSQL (dbh) "insert into testbundle (target_id, cfg_id, date) \
			   values ($target_id, $cfg_id, $datestamp) \
			   returning bundle_id")

let create_testrun dbh bundle_id mlib_id =
  one_result (PGSQL (dbh) "insert into testrun (bundle_id, mlib_id) \
			   values ($bundle_id, $mlib_id)
			   returning run_id")

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

let add_result dbh run_id test_id test_collate result =
  let result_str = string_of_result result in
  PGSQL (dbh) "insert into results (run_id, test_id, test_collate, result) \
	       values ($run_id, $test_id, $test_collate, $result_str)"

let read_sum_file dbh comp_id run_id sumfile =
  let gdb_comp_failed = Str.regexp "\\(^gdb compile failed, \\)" in
  let line_number = ref 0
  and current_expfile = ref "(unknown exp file)" in
  let current_expid = ref (expfile_id dbh !current_expfile)
  and fh = open_in sumfile in
  skip_header fh;
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
          Unknown -> ()
	| Begin_exp e ->
	    let stripped = expfile_strip e "testsuite" in
	    let exp_id = expfile_id dbh stripped in
	    current_expfile := e;
	    current_expid := exp_id
	| Result (res, test)
	| Test_error (res, test) ->
	    let test_id = test_id dbh comp_id !current_expid test in
	    add_result dbh run_id test_id (Int32.of_int !line_number) res
	end;
	incr line_number
    done
 with End_of_file -> close_in fh


let _ =
  Unix.putenv "PGDATABASE" "gcc_results";
  Unix.putenv "PGPASSWORD" "password";
  Unix.putenv "PGHOST" "localhost";
  Unix.putenv "PGPORT" "5432";
  let dbh = PGOCaml.connect () in
  
  let bundle_id = create_bundle dbh (target_id dbh "6502")
		  (config_id dbh "build.sh") (CalendarLib.Calendar.now ()) in

  let run_id = create_testrun dbh bundle_id (multilib_id dbh ".;") in
  
  read_sum_file dbh (component_id dbh "gcc") run_id
    "/home/jules/stuff-in-hiding/gcc-6502/testresults-2014-04-05-1/gcc.sum";
  
  let print_comp c =
    Printf.printf "%s\n" c in
  
  let compos = PGSQL(dbh) "select component from components" in
  List.iter print_comp compos;
  
  PGOCaml.close dbh
