#!/bin/bash
if [ "$#" -ne 4 ]; then
	echo "illegal number of parameters"
	echo "valid parameters: -root_dir -text_file -w -p"
	exit 1
fi
echo "root directory: $1 text file: $2 w: $3 p: $4"
SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
start=1
dir=$1
file=$2
w=$3
p=$4
q=$((w/2+1))	#number of external links per page
f=$((p/2+1))	#number of internal links per page
echo "q=$q f=$f"
html_str="<!doctype html>
<html>
<head>
    <title>HTML Page</title>
</head>
<body>

</body>
</html>"
if  ! [[ "$w" =~ ^[0-9]+$ && "$p" =~ ^[0-9]+$ ]]; then
        echo "w or p not an integer!"
	exit 1
fi

if  [ ! -d "$dir" ]; then
	echo "Root directory not found!"
	exit 1
else
	if [ "$(ls -A $dir)" ]; then
		echo "Root directory is not empty!"
		while true; do
    		read -p "Do you wish to purge directory?" yn
    		case $yn in
       		[Yy]* ) rm $dir/* -r; break;;	#-r kai gia fakelous
        	[Nn]* ) exit;;
       		* ) echo "Please answer yes or no.";;
    		esac
		done
	fi
fi

if [ ! -f "$file" ]; then
   	echo "Text file not found!"
	exit 1
fi
linecount=$(wc -l $file | cut -f1 -d' ')
if (( "$linecount" < 10000 )); then
   	echo "Text file has few lines!"
	#edw kanonika exit
fi

echo "Starting creation process!"

filenames=()
for ((i=1; i<=w; i++)); do	#ftia3e fakelous arxeia
	mkdir "$dir/site$i"
	#line=$(sed -n '1,50p' $file)	put line 1-50 of file to variable line
	cd $dir/site$i
	for ((j=1; j<=p; j++)); do		
		rand=$(($RANDOM%10000+1000))
		currtablesize=(${#filenames[@]})
		#echo $currtablesize
		for((l=0 ;l<$currtablesize; l++));do
			#echo "${filenames[$l]}"
			if [ "${filenames[$l]}" == "page$i"_"$rand.html" ];then
				rand=$(($RANDOM%10000+1000))
			fi
		done
		filenames+=("page$i"_"$rand.html")
		touch page$i"_"$rand.html
		echo "$html_str" >> page$i"_"$rand.html
	done
	cd -
done
echo "${filenames[@]}"
echo "number of elements: ${#filenames[@]}"
echo ""
echo ""
haslink=()
for ((i=0; i<${#filenames[@]}; i++)); do
	haslink[$i]=0
done
echo "haslink elems: ${haslink[@]}"
echo "haslink size: ${#haslink[@]}"
for ((i=1; i<=w; i++)); do	#gemise ta html arxeia apo to textfile
	external=()
	cd $dir
	for D in * ; do 
		#echo "$D"
    		case "site$i" in		#gia na vrw external pages gia to tade site apokleiw to idio to site alliws prosthetw ston pinaka external+=(*)
				(*"$D"*) ;;
				(*) cd $D
				for file in * ;do
					#echo $file
					external+=("/$D/$file")
				done
				cd ..
		esac
	done
	cd ..
	#echo "external links for site $i are: ${external[@]}"
	cd $dir/site$i
	echo "Entering folder $dir/site$i"
	files=(*)		#array me ta pages tou site i
	#echo "${files[@]}"
	for ((j=0; j<p; j++)); do
		k=$(($RANDOM%($linecount-2000)+1))		#Επιλέγουμε έναν τυχαίο αριθμό 1 < k < #lines in text_file - 2000
		m=$(($RANDOM%1000+1000))			#Επιλέγουμε έναν τυχαίο αριθμό 1000 < m < 2000
		offset=$(($k+$m))
		#echo "this is offset: $offset"					
		echo "Creating Page:--------> ${files[$j]} <----------"
		echo "k: $k"
		echo "m: $m"
		internal=()
		for file in "${files[@]}"; do
			case "/${files[$j]}/" in
				(*"/$file/"*) ;;
				(*) internal+=("/site$i/$file")
			esac
		done
		#echo "internal links for page ${files[$j]} are: ${internal[@]}"
		tempindexes=()
		links=()
		n=$((${#internal[@]}-1))
		tempindexes=($(seq 0 $n | shuf -n $f))
		#echo ${tempindexes[@]}
		t=$f
		for((l=0; l<$t ; l++)); do
			#echo "iteration: $l"
			links+=(${internal[${tempindexes[$l]}]})
		done
		tempindexes=()
		n=$((${#external[@]}-1))
		tempindexes=($(seq 0 $n | shuf -n $q))
		#echo ${tempindexes[@]}
		t=$q
		for((l=0; l<$t ; l++)); do
			#echo "iteration: $l"
			links+=(${external[${tempindexes[$l]}]})
		done

		echo "Randomly selected links: ${links[@]}"

		linkindexes=()
		randomizedlinks=()
		n=$((${#links[@]}-1))		
		linkindexes=($(seq 0 $n | shuf -n ${#links[@]}))
		for((l=0; l<${#links[@]} ; l++)); do
			#echo "iteration: $l"
			randomizedlinks+=(${links[${linkindexes[$l]}]})
		done
		
		echo "Randomized links: ${randomizedlinks[@]}"

		for((r=0; r<${#links[@]} ; r++)); do			#kathe fora pou vriskeis e3erxomeno link pros kapoia selida vaze tin selida oti exei incoming link
			#echo "${links[$r]}"
			for((t=0; t<${#filenames[@]}; t++))do
				#echo "${filenames[$t]}"
				if [[ "${links[$r]}" = *"${filenames[$t]}"* ]] && [ "${haslink[$t]}" != 1 ] ;then
					echo "found link to ${links[$r]} --- ${filenames[$t]} --- ${haslink[$t]}"	#contains filename
					haslink[$t]=1
				fi
			done
		done

		sed -i 6r<(sed "$k,$offset!d" $SCRIPTPATH/text_file) ${files[$j]}	#insert text inside body
		linkoffset=$((m/$((f+q))))
		echo "Offset between links is:$linkoffset"
		a=$linkoffset
		for((k=0; k<$(($f+$q)); k++)); do
			#echo "Will print $k time"
			echo "$k.Inserting link in line:$a"
			sed -i "$a"r<(echo "<p></p><p><a href=\"${randomizedlinks[$k]}\">link to ${randomizedlinks[$k]}</a></p><p></p>") ${files[$j]}	#insert links in m/f+q offsets
			let "a=a+linkoffset"
		done
		echo ""
		echo ""
	done
	cd ../..
done


#haslink[5]=0 for checking the incoming links functionality

echo "haslink elems: ${haslink[@]}"
echo "haslink size: ${#haslink[@]}"

allinc=1
for((i=0; i<${#haslink[@]} ; i++));do
	if [ "${haslink[$i]}" != 1 ];then
		allinc=0
	fi
done
if [ $allinc != 1 ];then
	echo "Not all pages have incoming links!!"
else
	echo "All pages have incoming links!!"
fi
echo "Finished creation process!"
