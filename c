#!/bin/sh

#Save default Field separator
SAVEIFS=$IFS
restoreIFS(){
    IFS=$SAVEIFS
}

#Trap in case of exit
trap restoreIFS INT

#Build Shortcuts
cFile(){
#Quickly compile and run C code
   cFile=$(echo "$1"|rev |cut -d"/" -f1|rev|cut -d. -f1)
   error=$(make "$cFile")
   #If no error then run executable
   if [ $? -eq 0 ]
   then
		#Create arguements string
	    args=" "
	    for i in $@
	    do
	        if [ $i != $1 ]
	        then
	            args="$args $i"

	        fi
	    done
    	#Run the executable
		./$cFile $args
   fi
   #If executable was made delete it
   if [ -f $cFile ]
   then
       rm $cFile
   fi
}

cppFile(){
#Quickly compile and run C++ code
   cFile=$(echo "$1"|rev |cut -d"/" -f1|rev|cut -d. -f1)
   # cFile=$(echo "$cFile"|cut -d. -f1)
   error=$(make -s "$cFile")
   #If no error run executable
   if [ $? -eq 0 ]
   then
		#Create arguements string
	    args=" "
	    for i in $@
	    do
	        if [ $i != $1 ]
	        then
	            args="$args $i"

	        fi
	    done
    	#Run the executable
		./$cFile $args
   fi
   #If executable was made delete it
   if [ -f $cFile ]
   then
       rm $cFile
   fi
}

javaFile(){
#Quickly run and compile java code
    #Compile the .java file


    error=$(javac $1)
    #If executable was generated run it
    if [ $? -eq 0 ];then
        cFile=$(echo $1|rev |cut -d"/" -f1|rev|cut -d. -f1)
        args=" "
	    #Create arguements string
	    for i in $@
	    do
	        if [ $i != $1 ]
	        then
	            args="$args $i"

	        fi
	    done
        #Run the executable
        java $cFile $args
    fi

    #Remove the .class file is generated
    if [ -f "$cFile.class" ]
    then
        rm "$cFile.class"
    fi
}

rustFile(){
	error=$(rustc $1)
	if [ $? -eq 0 ];then
		compiled=$(echo "$1"|rev|cut -d"/" -f1|rev|cut -d. -f1)
		args=" "
		for i in $@;do
			if [ "$i" != "$1" ];then
				args="$args $i"
			fi
		done
		./$compiled $args
	fi

	if [ -f "$compiled" ];then 
		rm "$compiled"
	fi
}


goFile(){
  refactoredCode=$(goimports "$1") 
  if [ -z "$refactoredCode" ];then
      echo "problem with goimports"
      return
  else
      echo "$refactoredCode" > "tempFile"
  fi
  mv "tempFile" $1
  for i in $@
  do
    if [ $i != $1 ]
    then
        args="$args $i"

    fi 
  done
  go run $1 $2 
}


latexFile(){
	curr=$(pwd)
	for i in $@;do
		parent=$(echo "$i"|rev|cut --complement -d/ -f1|rev)"/"
		filename=$(echo "$i"|rev|cut -d/ -f1|rev)
		cd $parent
		latexmk -pdf $filename
		echo "$i"
		# Clears temp files created by latexmk for creating pdfs from tex
		rm *.log *.fls *.synctex.gz *.fdb_latexmk *.aux *.dvi *.synctex\(busy\) 2> /dev/null
		cd $curr
	done
}

shellScript(){
	ext=$(echo "$1"|rev|cut -d. -f1|rev)
	line=$(head -1 $1)
	isShell=""
	case $line in
		"\#\!/bin/sh")isShell="true";;
		"\#\!/bin/zsh")isShell="true";;
		"\#\!/bin/bash")iisShell="true";;
		"\#\!/bin/fish")isShell="true";;
	esac

	if [ -z "$isShell" ];then
		echo "Filetype not supported"
		exit
	fi	

	if [ -x $1 ];then
		 $isPath$1
	else
		 chmod +x $1 && $1 $2
    fi
}

# Main Build Logic
if [ -z "$1" ]; then
    # display usage if no parameters given
    echo "Usage:   c <path/file_name of type c|c++|java|rust|python|go|tex>|any executable"
    echo "Example: c <path/file_name_1.c>"
    exit
else
	# set  Field separator
	IFS=$(printf "\n\b")

	# Get commanline args
	Args=" "
	for i in $@;do
	  if [ "$i" != "$1" ];then
	    echo "$i"
	    Args="$Args $i"
	  fi
	done

	#Check if absolute path is provided
	case "$1" in
	    /*) isPath=""
	        ;;

	    *)isPath="./"
	esac

	if [ -f "$1" ] ; then
	  case "${1%,}" in
	    *.c)	cFile $isPath"$1" "$Args" ;;
	    *.cpp)  cppFile $isPath"$1" "$Args" ;;
	    *.java) javaFile $isPath"$1" "$Args" ;;
	    *.go)   goFile $isPath"$1" "$Args" ;;
	    *.rs)   rustFile $isPath"$1" "$Args";;
	    *.py)   python $isPath"$1" "$Args";;
		*.tex)  for file in $@;do
					latexFile $isPath$file
	 	        done;;
	    *)    shellScript $isPath$1 $Args;; #if [ -x $1 ]; then
	           #;;
	  esac
	else
	      echo "'$1' - file does not exist"
	fi
fi

# restore $IFS
IFS=$SAVEIFS
