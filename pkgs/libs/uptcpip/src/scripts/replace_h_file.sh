sed -i 's/\([a-zA-Z0-9_]*\).h[>|"]$/bsd_&/g' `find -name *.c`
sed -i 's/\([a-zA-Z0-9_]*\).h[>|"]\(.*\)\*\/*$/bsd_&/g' 
