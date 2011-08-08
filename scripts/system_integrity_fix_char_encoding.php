<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: system_integrity_fix_char_encoding.php,v 1.1.4.1 2011/08/08 23:08:22 akarelia Exp $
*/

/**
* Script to replace the non-utf8 smart quotes chars by their regular counterpart chars and 
* if string is still invalid after replacement, perform charset conversion on string 
*
* IMPORTANT: SYSTEM MUST BE BACKEDUP BEFORE RUNNING THIS SCRIPT!!!
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.1.4.1 $
* @package MySource_Matrix
*/

error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {	
	print_usage();
	echo "ERROR: The directory you specified as the system root does not exist, or is not a directory.\n\n";

	exit;
}
if (ini_get('memory_limit') != '-1') ini_set('memory_limit', '-1');

$old_encoding = (isset($_SERVER['argv'][2])) ? $_SERVER['argv'][2] : '';
if (!$old_encoding || !isValidCharset($old_encoding)) {
	print_usage();	
	echo  "\nERROR: The charset you specified '$old_encoding', as system's old encoding as thrid parameter is not valid charset type.\n\n";

	exit;
}

$root_node_id = (isset($_SERVER['argv'][3])) ? $_SERVER['argv'][3] : 1;
if ($root_node_id == 1) {
	echo "\nWARNING: You are running this script on the whole system.\nThis is fine, but it may take a long time\n";
}

// Tables where the values are to fixed
//
// Array(
//		<db table> => Array(
//					"assetid" 	=> <fieldname containing the assetid>,
//					"contextid" => <fieldname containing the record's contextid>,
//					"value" 	=> <fieldname containing the record's value>,
//					"key" 		=> [<third field as part of the record's primary key>],
//				),
//		)		
$tables = Array(
			'sq_ast_attr_val'       => Array(
										'assetid' 		=> 'assetid', 
										'contextid'		=> 'contextid', 
										'value'			=> 'custom_val', 
										'key'			=> 'attrid',
									),
			'sq_ast_mdata_val'      => Array(
										'assetid' 		=> 'assetid', 
										'contextid'		=> 'contextid',
										'value'			=> 'value', 
										'key'			=> 'fieldid',
									),
			'sq_ast_mdata_dflt_val' => Array(
										'assetid'		=> 'assetid', 
										'contextid'		=> 'contextid', 
										'value'			=> 'default_val',
										'key'			=> '',
									),
			'sq_ast_attr_uniq_val'  => Array(
										'assetid'		=> 'assetid', 
										'contextid'		=> 'contextid',										
										'value'			=> 'custom_val',
										'key'			=> 'owning_attrid', 
									),
		);

// Define the replacement chars
//	
// Array(
//	<ASCII value of char to replace> 	=> <replacement char>
// )
$replacements = Array(
			'145' => "'",
			'146' => "'",
			'147' => "\"",
			'148' => "\"",			
			'150' => "-",
			'151' => "-",
			'133' => '...',
        );


require_once $SYSTEM_ROOT.'/core/include/init.inc';
define('SCRIPT_LOG_FILE', SQ_SYSTEM_ROOT.'/data/private/logs'.substr(__FILE__, strrpos(__FILE__,'/')).'.log');

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	echo "ERROR: The root password entered was incorrect\n";
	exit();
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed login in as root user\n", E_USER_ERROR);
}

echo "\nIMPORTANT: This script will replace all the smart quote chars by their regular counterpart chars. And if value string is still\n";
echo "invalid in the current system's charset then it performs charset conversion on string from older to current encoding\n";
echo "YOU MUST BACKUP YOUR SYSTEM BEFORE RUNNING THIS SCRIPT\n";
echo "Are you sure you want to proceed (Y/N)? \n";

$yes_no = rtrim(fgets(STDIN, 4094));
if (strtolower($yes_no) != 'y') {
	echo "\nScript aborted. \n";
	exit;
}

echo "Running the script in ";
for($countdown = 5; $countdown >= 0; $countdown--) {
	usleep(500000);
	echo $countdown." ";	
}

// No turning back now. Start char fixing.
fix_char_encoding($root_node_id, $tables, $replacements);

$GLOBALS['SQ_SYSTEM']->am->forgetAsset($root_user);

// End of Main program /////////////////////////////////


/**
* Fixes the char encoding in the given tables
*
* @param int 	$root_node		Assetid of rootnode, all childern of rootnode will be processed for char replacement
* @param array	$tables			DB tables and colunms info
* @param array	$replacements	Char replacement array 
								Array("ASCII Value of Char to be replced" => "New Char",)
*
* @return void
*/
function fix_char_encoding($root_node, $tables, $replacements)
{	
    $target_assetids = array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($root_node));
	if (!$target_assetids) {
		echo "\n\nNo assets found under root node #$root_node \n";
		return;
	}
	echo "\n\nNumber of assets to look into : ".count($target_assetids)." \n";
	
	$replacement_ords = array_keys($replacements);
	$start_time = microtime(TRUE);

	$errors = 0;	
	$invalid_records_count = 0;
	$records_fixed_count = 0;
	
	$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

    foreach($tables as $table => $fields) {
        $sql = 'SELECT '.trim(implode(',',$fields),',').' FROM '.$table;
		$results = MatrixDAL::executeSqlAssoc($sql);
	
		$count = 0;
		echo "Fixing characters in table $table .";
		foreach($results as $record) {			
			
			$value 		= isset($record[$fields['value']]) ? $record[$fields['value']] : NULL;
			$assetid 	= isset($record[$fields['assetid']]) ? $record[$fields['assetid']] : NULL;
			$key 		= isset($record[$fields['key']]) ? $record[$fields['key']] : NULL;
			$contextid 	= isset($record[$fields['contextid']]) ? $record[$fields['contextid']] : NULL;
			
			if (!in_array($assetid, $target_assetids) || is_null($value) || is_null($assetid) || is_null($contextid) || ($fields['key'] && is_null($key))) continue;

			echo ($count++)%100 === 0 ? '.' : '';
			
			//  If value does not contain any invalid char, we needn't proceed any further
			if (isValidValue($value)) {
				continue;
			}
			
			$update_required = FALSE;
			$invalid_records_count++;

			// Carryout the non-uft8 smart quotes replacment
			if (strtolower(SQ_CONF_DEFAULT_CHARACTER_SET) == 'utf-8') {
				for ($i = 0; $i < strlen($value); $i++) {
					$ord = ord($value[$i]);
					if (in_array($ord, $replacement_ords)) {
						$value[$i] = $replacements[$ord];
						$update_required = TRUE;					
					}
				}//end for
			}//end if
			
			// Check if the value is now valid
			if (!isValidValue($value)) {				
				// String might also contains the char(s) from older encoding which is/are not valid for current one
				// See if we can convert these without igonoring or interprating any chars
				$converted_value = @iconv($old_encoding, SQ_CONF_DEFAULT_CHARACTER_SET, $value);
				
				// If the converted value is valid in current encoding then its good to go
				// otherwise we'll just not use this value
				if ($converted_value != $value && isValidValue($converted_value)) {
					$value = $converted_value;
					$update_required = TRUE;
				}

			}

			if ($update_required) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				try {
					$sql = "UPDATE 
								$table 
							SET 
								".$fields['value']."=:value 
							WHERE 
								".$fields['assetid']."=:assetid".
								" AND ".$fields['contextid']."=:contextid".
								(!is_null($key) ? " AND ".$fields['key']."=:key" : "");
								
					
					$update_sql = MatrixDAL::preparePdoQuery($sql);
					
					MatrixDAL::bindValueToPdo($update_sql, 'value', $value);					
					MatrixDAL::bindValueToPdo($update_sql, 'assetid', $assetid);
					MatrixDAL::bindValueToPdo($update_sql, 'contextid', $contextid);
					if (!is_null($key)) MatrixDAL::bindValueToPdo($update_sql, 'key', $key);
					
					
					$execute = MatrixDAL::executePdoAssoc($update_sql);
					if (count($execute) > 1) {						
						$sql = str_replace(':assetid', $assetid, $sql);
						$sql = str_replace(':contextid', $contextid, $sql);
						$sql = str_replace(':contextid', $contextid, $sql);
						$sql = !is_null($key) ? str_replace(':key', $key, $sql) : $sql;						
						
						$errors++;
						$msg = "Executing query \"$sql\" will affect ".count($execute)." (more than 1) records! Ignoring this sql.";
						log_error_msg($msg);
						
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						continue;
					}
					
					$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
					$records_fixed_count++;

				} catch (Exception $e) {
					$error++;
					$msg = "Unexpected error occured while updating database: ".$e->getMessage();
					log_error_msg($msg);

					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					
				}
			} else {
				// This record contained invalid value. Either the invalid char(s) in it was/were not in the replacement array
				// or trying to carryout charset conversion (without losing any data) still resulted into invalid value
				// Hence replacement was not carried out
				$errors++;
				$msg = "Asset with ".$fields['assetid']."=#$assetid, ".
						(!is_null($key) ? $fields['key']."=#$key, and " : "and ").
						$fields['contextid']."=#$contextid in table $table ".
						"contains invalid char(s), which were not replaced because ".
						"either those invalid chars were not defined in the replacement array or the charset conversion was not successful";
				log_error_msg($msg);
			}

		}//end foreach
	
		echo " Done.\n";
		
    }//end foreach
    
	$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	
	unset($target_assetids);
	
	echo "\n";
	echo "Number of db records with invalid char(s): $invalid_records_count\n";
	echo "Number of db records replaced successfully: $records_fixed_count\n";
	echo "Total errors recorded: $errors \n";	
	echo "Total time taken to run the script: ".round(microtime(TRUE)-$start_time, 2)." second(s)\n";

	if ($errors > 0)	{
		echo "\nPlease check ".SCRIPT_LOG_FILE." file for errors\n\n";
	}
	echo "\n";

}//end replace_characters()


/**
* Check if the given value is valid for given charset
*
* @parm string	$value		String value to check
* @parm string 	$charset	Charset 
*
* return boolean
*/
function isValidValue($value, $charset=SQ_CONF_DEFAULT_CHARACTER_SET)
{	
	return $value == @iconv($charset, $charset."//IGNORE", $value);
}


/**
* Check if the given value is valid for given charset
*
* @parm string	$value		String value to check
* @parm string 	$charset	Charset 
*
* return boolean
*/
function isValidCharset($charset)
{	
	return 'test' == @iconv($charset, $charset, 'test');
}


/**
* Logs the error in script error log
*
*/
function log_error_msg($msg)
{	
	$msg = date('j-m-y h-i-s').": ".$msg."\n";
	file_put_contents(SCRIPT_LOG_FILE, $msg, FILE_APPEND);
}


/**
* Print the usage of this script
*
*/
function print_usage() 
{
	echo "\nThis script replaces all the non-utf8 smart quotes chars by their respective regular couterpart chars.";
	echo "\nIf string is still invalid in current charset encoding aftet the replacement then script will perform chaset";
	echo "\nconversion on string from previous charset to the current one.\n\n";
	
	echo "Usage: php ".basename(__FILE__)." <SYSTEM_ROOT> <OLD_CHARSET> [<ROOT_NODE>]\n\n";
	echo "\t<SYSTEM_ROOT>: 	The root directory of Matrix system.\n";	
	echo "\t<OLD_CHARSET>: 	Previous charset of the system. (eg. UTF-8, ISO-8859-1, etc)\n";
	echo "\t<ROOT_NODE>: 	Asstid of the rootnode (all children of the rootnode will be processed by the script).\n";
	
	echo "\nWARNING: IT IS STRONGLY RECOMMENDED THAT YOU BACKUP YOUR SYSTEM BEFORE RUNNING THIS SCRIPT\n\n";
	
}//end print_usage()

?>
