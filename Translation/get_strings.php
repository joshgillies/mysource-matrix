<?php
    $DIRNAME = dirname(dirname(__FILE__));
    $INCLUDE = [
        'core',
        'packages',
    ];

    $strings = [];
    $output  = [];
    foreach ($INCLUDE as $inc_dir) {
        exec('find '.$DIRNAME.'/'.$inc_dir.' -name lang_strings.xml', $output);
    }

    foreach ($output as $line) {
        $sxml = simplexml_load_file($line);
        foreach ($sxml->string as $string) {
            $code = (string) $string->attributes()->source;
            if (!empty($strings[$code]) && ($strings[$code] !== (string) $string)) {
                echo 'DUPLICATE: '.$code."\n";
                // echo 'DUPLICATE: '.$code.' (old: '.$strings[$code].', new: '.((string) $string).')'."\n";
            } else {
                $strings[$code] = (string) $string;
            }
        }
    }
    $strings = array_unique($strings);
    //print_r($strings);
    //print_r($output);

    $messages_subj = [];
    $messages_body = [];
    $output        = [];
    foreach ($INCLUDE as $inc_dir) {
        exec('find '.$DIRNAME.'/'.$inc_dir.' -name lang_messages.xml', $output);
    }

    foreach ($output as $line) {
        $sxml = simplexml_load_file($line);
        foreach ($sxml->message as $msg) {
            $code = (string) $msg->attributes()->type;
            if (!empty($messages_subj[$code])) {
                echo 'DUPLICATE MESSAGE: '.$code."\n";
            } else {
                $messages_subj[$code] = (string) $msg->subject;
                $messages_body[$code] = (string) $msg->body;
            }
        }
    }
    //print_r(implode(', ', array_keys($messages_subj)));

    $errors = [];
    $output = [];
    foreach ($INCLUDE as $inc_dir) {
        exec('find '.$DIRNAME.'/'.$inc_dir.' -name lang_errors.xml', $output);
    }

    foreach ($output as $line) {
        $sxml = simplexml_load_file($line);
        foreach ($sxml->error as $string) {
            $code = (string) $string->attributes()->code;
            if (!empty($errors[$code])) {
                echo 'DUPLICATE ERROR: '.$code.' (old: '.$errors[$code].', new: '.((string) $string).')'."\n";
            } else {
                $errors[$code] = (string) $string;
            }
        }
    }
    $errors = array_unique($errors);
    //print_r($errors);
    //print_r($output);
?>
