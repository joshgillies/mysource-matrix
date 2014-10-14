<?php
/**
 * +--------------------------------------------------------------------+
 * | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
 * | ABN 77 084 670 600                                                 |
 * +--------------------------------------------------------------------+
 * | IMPORTANT: Your use of this Software is subject to the terms of    |
 * | the Licence provided in the file licence.txt. If you cannot find   |
 * | this file please contact Squiz (www.squiz.com.au) so we may provide|
 * | you a copy.                                                        |
 * +--------------------------------------------------------------------+
 *
 * $Id: system_integrity_invalid_remaps.php,v 1.2 2013/09/09 00:08:59 ewang Exp $
 */

/**
 * Script to check remap entries, report and optionally delete invalid entries
 *
 *
 * @author  Edison Wang <ewang@squiz.com.au>
 * @version $Revision: 1.2 $
 * @package MySource_Matrix
 */

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
    trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = getCLIArg('system');
if (!$SYSTEM_ROOT) {
    echo "ERROR: You need to supply the path to the System Root\n";
    print_usage();
    exit(1);
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
    echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
    print_usage();
    exit(1);
}

if (ini_get('memory_limit') != '-1') {
    ini_set('memory_limit', '-1');
}
require_once ($SYSTEM_ROOT.'/core/include/init.inc');

$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);


/*
 * report expired remaps
 */
if(getCLIArg('expired_remaps')) {
    echo "\n## Expired Remaps ##\n";
    $time  = ts_iso8601(time());
    $sql = 'SELECT * FROM sq_ast_lookup_remap WHERE expires <= :time';
    $query = MatrixDAL::preparePdoQuery($sql);
    MatrixDAL::bindValueToPdo($query, 'time', $time);
    $affected_result =  MatrixDAL::executePdoAll($query);

    if(getCLIArg('execute') && !empty($affected_result)) {
        // delete them
        $count = deleteRemaps($affected_result);
        echo $count." remaps deleted\n";
    } else {
        // display them
        $count =count($affected_result);
        if (empty($count)) {
            echo "NONE\n";
        } else {
            echo "There are $count expired remap(s).\n";
        }
        printRemaps($affected_result);
    }
}


/*
 * report invalid old url remaps
 */
if (getCLIArg('invalid_old_url')) {
     echo "\n## Invalid Old URL ##\n";

    $sql = 'SELECT * FROM sq_ast_lookup_remap WHERE expires > :time OR expires is null';
    $query = MatrixDAL::preparePdoQuery($sql);
    $time = ts_iso8601(time());
    MatrixDAL::bindValueToPdo($query, 'time', $time);
    $result =  MatrixDAL::executePdoAll($query);

    $affected_result = Array();
    foreach ($result as $index => $url_info) {
        // ?a=xx style url
        if(strpos($url_info['url'], '//?a=') !== FALSE) {
            preg_match('/\?a=([0-9:]+)/', $url_info['url'], $matches);
            if (isset($matches[1])) {
                $url = $GLOBALS['SQ_SYSTEM']->am->getAssetURL($matches[1]);
                if (!empty($url)) {
                    $affected_result[] = $url_info;
                }
            }
        } else {
            // normal url
            $old_url_parts = parse_url($url_info['url']);
            $protocol = array_get_index($old_url_parts, 'scheme', NULL);
            if (!empty($protocol)) {
                $url = array_get_index($old_url_parts, 'host', '').array_get_index($old_url_parts, 'path', '');
                $url_asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($protocol, $url, TRUE, TRUE);
                if (!empty($url_asset)) {
                    $affected_result[] = $url_info;
                }
                $GLOBALS['SQ_SYSTEM']->am->forgetAsset($url_asset);
                unset($url_asset);
            }
        }
    }

    if (getCLIArg('execute') && !empty($affected_result)) {
        // delete them
        $count = deleteRemaps($affected_result);
        echo $count." remaps deleted\n";
    } else {
        // display them
        $count = count($affected_result);
        If (empty($count)) {
            echo "NONE\n";
        } else {
            echo "There are $count remap(s) with invalid old URL.\n";
        }

        printRemaps($affected_result);
    }
}


/*
 * report invalid new url remaps
 */
if(getCLIArg('invalid_new_url')) {
    echo "\n## Invalid New URL ##\n";

    $sql = 'SELECT * FROM sq_ast_lookup_remap WHERE expires > :time OR expires is null';
    $query = MatrixDAL::preparePdoQuery($sql);
    $time = ts_iso8601(time());
    MatrixDAL::bindValueToPdo($query, 'time', $time);
    $result =  MatrixDAL::executePdoAll($query);

    $affected_result = Array();
    $root_urls = explode("\n", SQ_CONF_SYSTEM_ROOT_URLS);
    foreach ($result as $index => $url_info) {
        $new_url_parts = parse_url($url_info['remap_url']);
        $protocol = array_get_index($new_url_parts, 'scheme', NULL);
        if (!empty($protocol)) {
            $url = array_get_index($new_url_parts, 'host', '').array_get_index($new_url_parts, 'path', '');
            //  does it match with root matrix url
            $is_matrix_url = FALSE;
            foreach($root_urls as $root_url) {
                if (strpos($url, $root_url.'/') === 0 || $url === $root_url){
                    $is_matrix_url = TRUE;
                    break;
                }
            }
             // if it's not a Matrix URL, no need to show warning
            if ($is_matrix_url) {
                 // if it's one of special url, no need to show warning
                if (strpos($url, '__data') === FALSE && strpos($url, '__lib') === FALSE && strpos($url, '__fudge') === FALSE) {
                    $url_asset = $GLOBALS['SQ_SYSTEM']->am->getAssetFromURL($protocol, $url, TRUE, TRUE);
                    $count_query = 'SELECT count(*) FROM sq_ast_lookup_remap WHERE url = :url ';
                    $count_result = MatrixDAL::preparePdoQuery($count_query);
                    MatrixDAL::bindValueToPdo($count_result, 'url', $url_info['remap_url']);
                    $remap_url = MatrixDAL::executePdoOne($count_result);

                    // if the new url is not an valid asset url, neither another redirect url, show warning
                    if (empty($url_asset) && empty($remap_url)) {
                        $affected_result[] = $url_info;
                    }
                   $GLOBALS['SQ_SYSTEM']->am->forgetAsset($url_asset);
                   unset($url_asset);
                }
            }
        }
    }

    if(getCLIArg('execute') && !empty($affected_result)) {
        // delete them
        $count = deleteRemaps($affected_result);
        echo $count." remaps deleted\n";
    } else {
        // display them
        $count = count($affected_result);
        if (empty($count)) {
            echo "NONE\n";
        } else {
            echo "There are $count remap(s) with invalid new URL.\n";
        }

        printRemaps($affected_result);
    }
}


/*
 * report long redirects remaps
 */
if(getCLIArg('redirect_chain')) {
    echo "\n## Redirect Chain ##\n";

    $db_type = _getDbType();

    $except_clause = 'EXCEPT';
    if($db_type === 'oci') {
        $except_clause = 'MINUS';
    }
    // find out all long redirects with magic query
    $sql = '(SELECT  a.* FROM sq_ast_lookup_remap a, sq_ast_lookup_remap b WHERE a.url = b.remap_url)
            UNION ALL
            (SELECT  c.* FROM sq_ast_lookup_remap c, sq_ast_lookup_remap d WHERE c.remap_url = d.url
                '.$except_clause.' SELECT c.* FROM sq_ast_lookup_remap c, sq_ast_lookup_remap d WHERE c.url = d.remap_url
            )';
    $query = MatrixDAL::preparePdoQuery($sql);
    $result =  MatrixDAL::executePdoAll($query);
    $sorted_result = Array();

    // now we have to sort them nicely to report
    while (count($result) > 0) {
        // grab a random element from remaining result
        $url_info = array_shift($result);
         // find chained elements redirecting from it, i.e trace in normal redirect order. e.g A->B->C->D, starting from B, it will find C and D
        $redirects_chain_from = recursiveTrace($url_info['remap_url'], $result, TRUE);
         // find chained elements redirecting to it, i.e trace in reversed order.  e.g A->B->C->D, starting from B, it will find A
        $redirects_chain_to = recursiveTrace($url_info['url'], $result, FALSE);
        // add the current element to it
        array_push($redirects_chain_to, $url_info);

        // now we found a complete chain of redirects...
        $sorted_result[] = array_merge($redirects_chain_to, $redirects_chain_from);
    }


    if(getCLIArg('execute') && !empty($sorted_result)) {
        // ignore infinite loop, those loops have to be broken with other options first
        foreach ($sorted_result as $index => $data) {
            if($data[0]['url'] === $data[ count($data) - 1]['remap_url']){
                unset($sorted_result[$index]);
            }
        }
          // break them
        $chain_count = count($sorted_result);
        $count = breakRemapChains($sorted_result);
        echo $count." short remaps added to replace $chain_count chains\n";
    } else {
        // display them
        $count =count($sorted_result);
        if (empty($count)) {
            echo "NONE\n";
        } else {
            echo "There are $count redirect chain(s) found.\n";
        }

        foreach ($sorted_result as $index => $data) {
            echo '- chain '.$index;
            if($data[0]['url'] === $data[ count($data) - 1]['remap_url']){
                echo " !! Infinite loop detected !! you have to use other options to break the loop first";
            }
            echo "\n";
            printRemaps($data);
        }
    }
}



/*
 *  helper functions
 */


function _getDbType()
{
        $dbtype = MatrixDAL::GetDbType();

        if ($dbtype instanceof PDO) {
                $dbtype = $dbtype->getAttribute(PDO::ATTR_DRIVER_NAME);
        }
        return strtolower($dbtype);
}


/**
 * recursively trace remaps and returns a redirect chain array
 * e.g remaps A->B->C,  given url B, and normal order, it will return B->C.
 *
 * @params $url string the url to trace with
 * @params $result array the global results array to trace in
 * @params $trace_order bool if it's true, trace in normal redirect order, false, trace in reversed order.
 *
 * @return null
 */
function recursiveTrace ($url, &$result, $trace_order)
{

    $long_redirects = Array();
    foreach ($result as $index => $url_info)  {
        // found a redirect from it
        if($trace_order && $url_info['url'] ===  $url) {
            unset($result[$index]);
            $long_redirects = recursiveTrace($url_info['remap_url'], $result, TRUE);
            array_unshift($long_redirects, $url_info);
            return $long_redirects;
        } elseif (!$trace_order && $url_info['remap_url'] ===  $url) {
            // found a redirect to it
            unset($result[$index]);
            $long_redirects = recursiveTrace($url_info['url'], $result, FALSE);
            array_push($long_redirects, $url_info);
            return $long_redirects;
        }
    }

    return $long_redirects;
}


/**
 * print remaps
 *
 * @params $results array
 *
 * @return null
 */
function printRemaps($results)
{
    foreach ($results as $data){
        echo $data['url'].' => '.$data['remap_url']."\n";
    }
}


/**
 * delete remaps
 *
 * @params $remaps array
 *
 * @return null
 */
function deleteRemaps($remaps)
{
    $count = 0;
    // chunck to small batches, so oracle can accept the sql
    foreach (array_chunk($remaps, 100) as $chunk) {
        $GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
        $GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
        $urls = Array();
        foreach($chunk as $data) {
            $urls[]  =MatrixDAL::quote($data['url']);
        }
        $remaps_sql = implode(', ', $urls);
        $sql = 'DELETE FROM sq_ast_lookup_remap WHERE url in ('.$remaps_sql.')';
        try {
            $count += MatrixDAL::executeSql($sql);
        } catch (Exception $e) {
            throw new Exception('Unable to delete remaps due to database error: '.$e->getMessage());
        }//end try-catch
        $GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
        $GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
    }
    return $count;
}


/**
 * break long redirect chain to short remaps
 *
 * @params $sorted_results array
 *
 * @return null
 */
function breakRemapChains($sorted_results)
{
    $rm = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
    $count = 0;
    foreach ($sorted_results as $chain) {
        // delete all remaps in the chain
        deleteRemaps($chain);
        // add new short remaps to last remap url
        $last_remap = $chain[count($chain) - 1];
        foreach ($chain as $remaps) {
            // using all the settings of last remap
            $rm->addRemapURL($remaps['url'], $last_remap['remap_url'], $last_remap['expires'], $last_remap['never_delete'], $last_remap['auto_remap']);
            $count++;
        }
    }
    return $count;
}


/**
 * Get CLI Argument
 * Check to see if the argument is set, if it has a value, return the value
 * otherwise return true if set, or false if not
 *
 * @params $arg string argument
 *
 * @return string/boolean
 * @author Matthew Spurrier
 */
function getCLIArg($arg)
{
    return (count($match = array_values(preg_grep("/--" . $arg . "(\=(.*)|)/i",$_SERVER['argv']))) > 0 === TRUE) ? ((preg_match('/--(.*)=(.*)/',$match[0],$reg)) ? $reg[2] : true) : false;

}//end getCLIArg()


/**
 * Print the usage of this script
 *
 * @return void
 */
function print_usage()
{
    echo "\nThis script checks for invalid remap entries, report and optionally delete them\n";

    echo "Usage: php ".basename(__FILE__)." --system=<SYSTEM_ROOT>  [--expired_remaps] [--invalid_old_url] [--redirect_chain]  [--invalid_new_url] [--execute]\n\n";
    echo "\t<SYSTEM_ROOT> : The root directory of Matrix system.\n";
    echo "\t[--expired_remaps]  : Find expired remaps.\n";
    echo "\t[--invalid_old_url] : Find rubbish remaps that have old URL matching to existing Matrix assets.\n";
    echo "\t[--redirect_chain]  : Find long redirect chains, break it to short remaps if execute is specified.  e.g A->B->C will be broken to A->C, B->C.\n";
    echo "\t[--invalid_new_url] : Find remaps with invalid new URL. e.g A->B where B is not a valid asset URL neither have another remap for it. It will only look for short remaps.\n";
    echo "\t[--execute]         : Delete the invalid remaps found in above reports. If it's redirect chains, break it to short remaps.\n";
    echo "\tNote: For best results, remove expired remaps and invalid old url remaps first,  and then break redirect chain URLs(optional), finally delete invalid new URLs.\n";
}//end print_usage()

?>
