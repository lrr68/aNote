#!/bin/sh

#-Simple script for taking notes in the terminal.
#-Notes are saved in $NOTES_PATH directory or $HOME/.notes if $NOTES_PATH is not set
#-run 'anote -h' for help

#reset for getopts
OPTIND=1

[ "$NOTES_PATH" ] || NOTES_PATH="$HOME/.notes"
# Assume there is a trailing /, remove it and add it again.
# if there is no trailing / it's simply going to add it
# not so good idea, i know it, but it's the way i could write
# it in POSIX shell.
NOTES_PATH="${NOTES_PATH%/}/"
#create NOTES_PATH directory if it doesn't exists
[ -d "$NOTES_PATH" ] ||  mkdir $NOTES_PATH

WHICH_NOTES=''
OPERATION='a'
TAG=''
NOTE=''
PATTERN=''
INVALID=false

add_note()
{
	if ! [ "$NOTE" = '' ]; then
		[ ! -f $NOTES_PATH.notes$TAG ] && touch $NOTES_PATH.notes$TAG
		#gets the last note number from file
		read NOTE_NUMBER<<<$(awk 'END{print $1}' $NOTES_PATH.notes$TAG | sed s/~//g)
		NOTE_NUMBER=$((NOTE_NUMBER+1))
		DATE=$(date +%Y-%m-%d)
		echo "~$NOTE_NUMBER	$DATE	'$NOTE'">>$NOTES_PATH.notes$TAG
	else
		echo "error: empty note"
	fi
}

list_notes()
{
	if [ "$TAG" ]; then
		WHICH_NOTES=".notes$TAG"
	else
		# dmenu prompt to select tag if there are any .notes files
		[ "$(ls -a $NOTES_PATH | grep ^.notes)" ] && WHICH_NOTES=.notes$(ls -a $NOTES_PATH | grep ^.notes | awk -F " " '{print substr($1,7)}'| dmenu -l 10)
	fi

	if test -f $NOTES_PATH$WHICH_NOTES; then
		awk -F "\t" '{print $1". " $3}' $NOTES_PATH$WHICH_NOTES
	else
		[ "$TAG" ] && echo "No notes tagged $TAG" || echo "No notes"
	fi
}

search_notes()
{
	[ "$PATTERN" ] || (echo "Nothing to search for. Exiting" && return)
	if [ $TAG ]; then
		WHICH_NOTES=".notes$TAG"; shift
		if test -f $NOTES_PATH$WHICH_NOTES; then
			 grep "$PATTERN" $NOTES_PATH$WHICH_NOTES | awk -F "\t" '{print $1. "\033[34m "$3"\033[0m"}'
		else
			echo 'No Notes'
		fi
	else
		for list in $(ls -a $NOTES_PATH | grep .notes); do
			FOUND=$(grep "$PATTERN" $NOTES_PATH$list | awk -F "\t" '{print $1. "\033[34m "$3"\033[0m"}')
			[ "$FOUND" ] && 
				echo "Found in $list" && 
				echo $FOUND
		done
	fi

}

show_help()
{
	echo 'aNote [-t] [SOME TAG] [SOME NOTE]: Adds a note to a specific Tag. If no tag is informed adds to general notes'
	echo 'aNote -l [SOME TAG]:Prompts with a dmenu to select notes file to list notes from
					-d flag to list general notes'
	echo 'aNote -s [SOMETHING]: Search for notes with SOMETHING. Date and numbers are valid'
	echo 'aNote -r [TAG]: prompts with a dmenu to select note to delete. if no tag is informed, prompt user for tags via dmenu'
	echo 'aNote -d: short for -t general'
}

remove_note()
{
	if [ "$TAG" ]; then
		WHICH_NOTES=".notes$TAG";
	else
		WHICH_NOTES=.notes$(ls -a $NOTES_PATH | grep ^.notes | awk -F " " '{print substr($1,7)}'| dmenu -l 10)
	fi

	if test -f "$NOTES_PATH$WHICH_NOTES"; then
		NOTE=$(cat $NOTES_PATH$WHICH_NOTES | sed "s/	/ /g" | dmenu -l 10 | awk -F " " '{print $1}')
		if [ "$NOTE" ];then
			sed '/'"$NOTE"'/d' $NOTES_PATH$WHICH_NOTES > $NOTES_PATH$WHICH_NOTES-aux
			mv $NOTES_PATH$WHICH_NOTES-aux $NOTES_PATH$WHICH_NOTES
			ALL_NOTES=$(cat $NOTES_PATH$WHICH_NOTES)
			if [ ! "$ALL_NOTES" ]; then
				rm $NOTES_PATH$WHICH_NOTES
			fi
		fi
	else
		echo 'No Notes'
	fi
}


while getopts ":ds:hlrt:" opt; do
	#parse arguments before calling any function
	case $opt in
		d) TAG='general'
			;;
		s) OPERATION='s'
			PATTERN="$OPTARG"
			;;
		h) OPERATION='h'
			;;
		l) OPERATION='l'
			TAG="$2"
			;;
		r) OPERATION='r'
			;;
		t) TAG="$OPTARG"
			;;
		\?) echo "invalid option: -$OPTARG" >&2
			show_help
			;;
	esac
	[ "$opt" == ":" ] && echo "Missing argument for $OPTARG" && INVALID=true
done

shift $((OPTIND -1))
# leftover arg is note itself
NOTE=$@

if [ ! "$INVALID" == "true" ]
then
	case $OPERATION in
			# no tag in add means general
		'a') [ "$TAG" ] || TAG='general'
			 add_note $NOTE $TAG
			;;
		'l') list_notes
			;;
		's') search_notes
			;;
		'r') remove_note
			;;
		'h') show_help
			;;
	esac
fi
