{
	if($0 ~ /^\\input\{/){
		iname = $1;
		gsub("\\\\input{", "", iname);
		gsub("}", "", iname);
		printf("%s: %s.tex\n", fname, iname);
	}
}
