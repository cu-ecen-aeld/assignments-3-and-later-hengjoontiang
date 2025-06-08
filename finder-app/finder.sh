#!/usr/bin/sh
<<comments
Write a shell script finder-app/finder.sh as described below:

    Accepts the following runtime arguments: the first argument is a path to a directory on the filesystem, referred to below as filesdir; the second argument is a text string which will be searched within these files, referred to below as searchstr

    Exits with return value 1 error and print statements if any of the parameters above were not specified

    Exits with return value 1 error and print statements if filesdir does not represent a directory on the filesystem

    Prints a message "The number of files are X and the number of matching lines are Y" where X is the number of files in the directory and all subdirectories and Y is the number of matching lines found in respective files, where a matching line refers to a line which contains searchstr (and may also contain additional content).
comments

#1 checks the number of arguments ; it mus be 2
if [ $# -ne 2 ]; then 
	echo "There should be exactly 2 arguments."
	echo "Argument1 for the folder, Argument2 for the string to search in the folder files"
	exit 1
fi

#2 checks that arg 1 is a folder, else highlight 
if [ ! -d $1 ]; then
	echo "Argument 1 should be a folder"
	exit 1
fi

#3 $1 is a folder, now we proceed to search through the files in the folder to look for instances of the search string
var_num_files_in_folder=$(find $1 -type f | wc -l )
var_num_of_match_lines=$(grep -R $2 $1  | wc -l ) 
echo "The number of files are ${var_num_files_in_folder} and the number of matching lines are ${var_num_of_match_lines}"
#grep -R $2 $1
