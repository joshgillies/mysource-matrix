#!/bin/bash

for asset_dir in `find . -name 'edit_interface_screen_*.xml' -exec dirname {} \; | sort -u`;
do
	echo "$asset_dir"
	
    pushd "$asset_dir" > /dev/null
    mkdir -p locale/en

	for ei_file in `ls -1 edit_interface_screen_*.xml`;
	do
		echo "    $ei_file"

		lang_file=`echo "$ei_file" | sed 's/edit_interface_screen_/lang_screen_/'`
		cp "$ei_file" "locale/en/$lang_file"

		cat "$ei_file" \
			| fgrep -v '<display_name' \
			| fgrep -v '<note' \
			> ".$ei_file.tmp"
		mv ".$ei_file.tmp" "$ei_file"

		pushd locale/en > /dev/null
		cat "$lang_file" \
			| fgrep -v '<assetid' \
			| fgrep -v '<boolean' \
			| fgrep -v '<colour' \
			| fgrep -v '<datetime' \
			| fgrep -v '<duration' \
			| fgrep -v '<email' \
			| fgrep -v '<email_format' \
			| fgrep -v '<float' \
			| fgrep -v '<html_width' \
			| fgrep -v '<int' \
			| fgrep -v '<option_list' \
			| fgrep -v '<parameter_map' \
			| fgrep -v '<password' \
			| fgrep -v '<selection' \
			| fgrep -v '<serialise' \
			| fgrep -v '<text' \
			| fgrep -v '<wysiwyg' \
			| sed 's/ write_access="[^"]*"//' \
			| sed 's/ read_access="[^"]*"//' \
			| sed 's/ show_if="[^"]*"//' \
			| sed 's/ limbo_access="[^"]*"//' \
			| sed 's/ format="[^"]*"//' \
			| sed 's/ keyword="[^"]*"//' \
			| sed 's/ hidden="[^"]*"//' \
			> ".$lang_file.tmp"
		mv ".$lang_file.tmp" "$lang_file"

		popd > /dev/null

	done;
	echo

    popd > /dev/null

done;

