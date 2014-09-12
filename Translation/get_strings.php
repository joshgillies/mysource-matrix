<?php
    $DIRNAME = dirname(dirname(__FILE__));
    $INCLUDE = [
        'core',
    ];

    exec('find '.$DIRNAME.'/packages -mindepth 1 -maxdepth 1 -type d', $pkgs);
    foreach ($pkgs as $pkg) {
        $INCLUDE[] = str_replace($DIRNAME.'/', '', $pkg);
    }

    $all_strings = [];
    foreach ($INCLUDE as $inc_dir) {
        $all_strings[$inc_dir] = [];
        $strings =& $all_strings[$inc_dir];

        $output  = [];
        exec('find '.$DIRNAME.'/'.$inc_dir.' -name lang_strings.xml', $output);

        foreach ($output as $line) {
            $sxml = simplexml_load_file($line);
            foreach ($sxml->string as $string) {
                $code = (string) $string->attributes()->source;
                $strings[] = trim((string) $string);
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

        echo str_pad($inc_dir, 35).' : '.number_format(count($strings))."\n";
    }//end for

    echo str_pad('Total', 35).' : '.number_format(array_reduce($all_strings, function($carry, $next) {
        return $carry + count($next);
    }, 0))."\n";
    //print_r($errors);
    //print_r($output);
?>
