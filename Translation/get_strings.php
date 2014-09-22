<?php
    $DIRNAME = dirname(dirname(__FILE__));
    $INCLUDE = [
        'core',
        'packages/cms',
        'packages/search',
    ];

    exec('find '.$DIRNAME.'/packages -mindepth 1 -maxdepth 1 -type d', $pkgs);
    foreach ($pkgs as $pkg) {
        $INCLUDE[] = str_replace($DIRNAME.'/', '', $pkg);
    }
    $INCLUDE = array_unique($INCLUDE);

    $all_strings = [];
    $strings_xml = [];
    foreach ($INCLUDE as $inc_dir) {
        $all_strings[$inc_dir] = [];
        $strings_xml[$inc_dir] = [];
        $strings =& $all_strings[$inc_dir];

        $output  = [];
        exec('find '.$DIRNAME.'/'.$inc_dir.' -name lang_strings.xml', $output);

        foreach ($output as $line) {
            $sxml = simplexml_load_file($line);
            foreach ($sxml->string as $string) {
                $code = (string) $string->attributes()->source;
                $strings[] = trim((string) $string);
                $strings_xml[$inc_dir][$code] = trim((string) $string);
            }
        }

        $output = [];
        exec('find '.$DIRNAME.'/'.$inc_dir.' -name lang_messages.xml', $output);

        foreach ($output as $line) {
            $sxml = simplexml_load_file($line);
            foreach ($sxml->message as $msg) {
                $code = (string) $msg->attributes()->type;
                $string[] = trim((string) $msg->subject);
                $string[] = trim((string) $msg->body);
            }
        }

        $output = [];
        exec('find '.$DIRNAME.'/'.$inc_dir.' -name lang_errors.xml', $output);

        foreach ($output as $line) {
            $sxml = simplexml_load_file($line);
            foreach ($sxml->error as $string) {
                $code = (string) $string->attributes()->code;
                $strings[] = trim((string) $string);
            }
        }

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
        }

        $strings = array_values($strings);
        $strings = array_unique($strings);
        if ($inc_dir !== 'core') {
            $strings = array_diff($strings, $all_strings['core']);
        }

        //echo str_pad($inc_dir, 35).' : '.number_format(count($strings))."\n";
/*
        $output = [];
        exec('find '.$DIRNAME.'/'.$inc_dir.' -name *.inc -or -name *.js', $output);
        foreach ($output as $output_file) {
            $lines = file($output_file);
            //echo $output_file."\n";
            $all_found = FALSE;
            foreach ($lines as $num => &$line) {
                $regex = '/translate\\(\'(([^\']|\\\\\')*?)\'\\)/';
                preg_match_all($regex, $line, $matches, PREG_SET_ORDER);
                $used_matches = [];
                foreach ($matches as $match) {
                    $all_found = TRUE;
                    if (in_array($match[1], $used_matches)) {
                        continue;
                    }
                    $used_matches[] = $match[1];
                    //echo str_pad($num, 6).' '.$match[1]."\n";

                    if (isset($strings_xml['core'][$match[1]])) {
                        $line = str_replace('\''.$match[1].'\'', '\''.str_replace('\'', '\\\'', $strings_xml['core'][$match[1]]).'\'', $line)."\n";
                    } else if (isset($strings_xml['packages/cms']) && isset($strings_xml['packages/cms'][$match[1]])) {
                        $line = str_replace('\''.$match[1].'\'', '\''.str_replace('\'', '\\\'', $strings_xml['packages/cms'][$match[1]]).'\'', $line)."\n";
                    } else if (isset($strings_xml['packages/search']) && isset($strings_xml['packages/search'][$match[1]])) {
                        $line = str_replace('\''.$match[1].'\'', '\''.str_replace('\'', '\\\'', $strings_xml['packages/search'][$match[1]]).'\'', $line)."\n";
                    } else if (isset($strings_xml[$inc_dir][$match[1]])) {
                        $line = str_replace('\''.$match[1].'\'', '\''.str_replace('\'', '\\\'', $strings_xml[$inc_dir][$match[1]]).'\'', $line)."\n";
                    } else {
                        echo '! '.$output_file.' '.$num.' '.$match[1]."\n";
                    }
                }//end foreach
            }

            if ($all_found) {
                file_put_contents($output_file, implode('', $lines));
            }
        }
    }//end for
*/
    /*echo str_pad('Total', 35).' : '.number_format(array_reduce($all_strings, function($carry, $next) {
        return $carry + count($next);
    }, 0))."\n";
    */ //print_r($errors);
    //print_r($output);
?>
