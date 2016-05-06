<?php
    $DIRNAME = dirname(dirname(__FILE__));
    $INCLUDE = [
        'core',
        'packages/cms',
        'packages/search',
        'fudge',
        'install'
    ];

    exec('find '.$DIRNAME.'/packages -mindepth 1 -maxdepth 1 -type d', $pkgs);
    foreach ($pkgs as $pkg) {
        $INCLUDE[] = str_replace($DIRNAME.'/', '', $pkg);
    }
    $INCLUDE = array_unique($INCLUDE);

    $all_strings = [];
    $strings_tr  = [];
    $js_strings  = [];
    $errs_avail  = [];
    foreach ($INCLUDE as $inc_dir) {
        $all_strings[$inc_dir] = [];
        $strings_tr[$inc_dir]  = [];
        $strings =& $all_strings[$inc_dir];

        $output = [];
        exec('find '.$DIRNAME.'/'.$inc_dir.' -name *.inc -or -name *.js', $output);
        foreach ($output as $output_file) {
            $lines = file($output_file);
            $all_found = FALSE;
            foreach ($lines as $num => &$line) {
                $regex = '/translate\\(\'(([^\']|\\\\\')*?)\'\\)/';
                preg_match_all($regex, $line, $matches, PREG_SET_ORDER);
                $used_matches = [];
                foreach ($matches as $match) {
                    $all_found = TRUE;
                    $match[1]  = str_replace('\\\'', '\'', $match[1]);
                    $match[1]  = str_replace('\\"', '"', $match[1]);

                    $strings[] = $match[1];
                    if (strpos($line, 'js_translate') !== FALSE) {
                        $js_strings[] = $match[1];
                    }
                    $strings_tr[$inc_dir][] = $match[1];
                }//end foreach
            }
        }//end for

        $output = [];
        exec('find '.$DIRNAME.'/'.$inc_dir.' -name edit_interface_*screen*.xml', $output);

        foreach ($output as $line) {
            $sxml = simplexml_load_file($line);
            foreach ($sxml->xpath('//display_name') as $string) {
                $code = (string) $string->attributes()->code;
                $strings[] = trim((string) $string);
            }

            foreach ($sxml->xpath('//note') as $string) {
                $code = (string) $string->attributes()->code;
                $strings[] = trim((string) $string);
            }

            foreach ($sxml->xpath('//boolean') as $string) {
                $code = (string) $string->attributes()->true_text;
                $strings[] = trim((string) $string);
                $code = (string) $string->attributes()->false_text;
                $strings[] = trim((string) $string);
            }
        }
        $strings_tr[$inc_dir] = array_values($strings_tr[$inc_dir]);
        $strings_tr[$inc_dir] = array_unique($strings_tr[$inc_dir]);

        $js_strings = array_values($js_strings);
        $js_strings = array_unique($js_strings);

        $strings = array_values($strings);
        $strings = array_unique($strings);
        if ($inc_dir !== 'core') {
            $strings = array_diff($strings, $all_strings['core']);
        }

        echo str_pad($inc_dir, 35).' : '.str_pad(number_format(count($strings)), 6)."\n";
    }//end for

    echo str_pad('Total', 35).' : '.number_format(array_reduce($all_strings, function($carry, $next) {
        return $carry + count($next);
    }, 0))."\n";

    $common_header = Array(
        'Project-Id-Version: 1.0',
        'Language: en',
        'MIME-Version: 1.0',
        'Content-Type: text/plain; charset=UTF-8',
        'Content-Transfer-Encoding: 8bit',
    );
    foreach ($INCLUDE as $inc_dir) {
        $base_file  = '';
        $base_file .= 'msgid ""'."\n";
        $base_file .= 'msgstr ""'."\n";
        foreach ($common_header as $value) {
            $base_file .= '"'.str_replace('"', '\\"', $value).'\\n"'."\n";
        }
        $base_file .= "\n";

        $file_index = 1;
        //foreach (array_chunk($all_strings[$inc_dir], 150) as $chunk) {
            $chunk = $all_strings[$inc_dir];
            //if (count($all_strings[$inc_dir]) <= 150) {
                $file_name = str_replace('/', '_', $inc_dir).'.po';
            //} else {
            //    $file_name = str_replace('/', '_', $inc_dir).'_'.$file_index.'.po';
            //}
            $file = $base_file;
            foreach ($chunk as $string) {
                if (array_search($string, $js_strings) !== FALSE) {
                    $file .= '#, js-translate'."\n";
                }

                $file .= 'msgid "'.str_replace('"', '\\"', $string).'"'."\n";
                $file .= 'msgstr ""'."\n\n";
            }

            file_put_contents(dirname(__FILE__).'/'.$file_name, $file);
            $file_index++;
        //}
    }
    //print_r($errors);
    //print_r($output);
?>
