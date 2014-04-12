export PGHOST=localhost
export PGPORT=5432
export PGUSER=$(whoami)
export PGPASSWORD=password
#export UNIX_DOMAIN_SOCKET_DIR=/var/run/postgresql
export PGDATABASE='gcc_results'
ocamlfind ocamlc -package pgocaml,pgocaml.syntax,fileutils -linkpkg -syntax camlp4o resultdb.ml -o resultdb
