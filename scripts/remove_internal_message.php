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
* $Id: remove_internal_message.php,v 1.12 2013/02/21 23:45:48 cupreti Exp $
*
*/

/**
* Delete internal messages
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix
*/
if (defined('E_STRICT')) {
	error_reporting(E_ALL ^ E_DEPRECATED ^ E_STRICT);
} else {
	if (defined('E_DEPRECATED')) {
		error_reporting(E_ALL ^ E_DEPRECATED);
	} else {
		error_reporting(E_ALL);
	}
}

if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

require_once 'Console/Getopt.php';

$shortopt = 's:p:f:t:y:u:a:';
$longopt = Array('quiet', 'show-query-only');

$con = new Console_Getopt();
$args = $con->readPHPArgv();
array_shift($args);
$options = $con->getopt($args, $shortopt, $longopt);

if (empty($options[0])) usage();

// Get root folder and include the Matrix init file, first of all
$SYSTEM_ROOT = '';
foreach ($options[0] as $index => $option) {
	if ($option[0] == 's' && !empty($option[1])) {
		$SYSTEM_ROOT = $option[1];
		unset($options[0][$index]);
	}
}//end foreach

if (empty($SYSTEM_ROOT)) {
	echo "ERROR: You need to supply the path to the System Root as the first argument\n";
	usage();
}

if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
	echo "ERROR: Path provided doesn't point to a Matrix installation's System Root. Please provide correct path and try again.\n";
	usage();
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

$PERIOD = '';
$USER_FROM = '';
$USER_TO = '';
$MSG_TYPE = '';
$MSG_STATUS = '';
$SHOW_QUERY_ONLY = FALSE;
$QUIET = FALSE;
$ASSETIDS = array();

foreach ($options[0] as $option) {

	switch ($option[0]) {

		case 'a':
			if (empty($option[1])) usage();
			$ASSETIDS[] = $option[1];
		break;

		case 'p':
			if (empty($option[1])) usage();
			$matches = Array();
			if (!preg_match('|^(\d+)([hdwmy])$|', $option[1], $matches)) {
				usage();
			}

			$time_num = (int) $matches[1];
			$time_units = '';
			switch ($matches[2]) {
				case 'h' :
					$time_units = 'hour';
				break;
				case 'd' :
					$time_units = 'day';
				break;
				case 'w' :
					$time_units = 'week';
				break;
				case 'm' :
					$time_units = 'month';
				break;
				case 'y' :
					$time_units = 'year';
				break;
			}
			if ($time_num > 1) $time_units .= 's';

			$PERIOD = date('Y-m-d H:i:s', strtotime('-'.$time_num.' '.$time_units));
		break;

		case 'f':
		case 't':
			$value = (string) $option[1];
			if ($value != '0' && empty($value)) usage();
			$var = ($option[0] == 'f') ? '$USER_FROM' : '$USER_TO';
			eval($var.' = $option[1];');
		break;

		case 'y':
			if (empty($option[1])) usage();
			$MSG_TYPE = $option[1];
		break;

		case 'u':
			if (empty($option[1])) usage();
			//if (!in_array($option[1], Array('U', 'D', 'R'))) usage();
			if (!preg_match('/[U|D|R]{1}(:[U|D|R]{1}(:[U|D|R])?)?/', $option[1])) usage();
			$MSG_STATUS = $option[1];
		break;

		case '--quiet':
			$QUIET = TRUE;
		break;

		case '--show-query-only':
			$SHOW_QUERY_ONLY = TRUE;
		break;

		default:
			echo 'Invalid option - '.$option[0];
			usage();
		break;

	}//end switch

}//end foreach arguments

if (empty($PERIOD)) usage();

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
$db =& $GLOBALS['SQ_SYSTEM']->db;

purge_internal_message($PERIOD, $USER_FROM, $USER_TO, $MSG_TYPE, $MSG_STATUS, $ASSETIDS);

if ($SHOW_QUERY_ONLY) {
	$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
} else {
	$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
}
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();


/**
* Remove internal messages
*
* @param string	$period		The period to remove internal messages before
* @param string	$user_from	The userid that the message is sent from
* @param string	$user_to	The userid that the message is sent to
* @param string	$msg_type	The type of internal message to remove, e.g. asset.linking.create, cron.*
* @param string	$msg_status	The status of internal message to remove, e.g. U or D
* @param array	$assetids	The asset id's to delete messages for.
*
* @return void
* @access public
*/
function purge_internal_message($period, $user_from='', $user_to='', $msg_type='', $msg_status='', $assetids=array())
{
	global $db, $QUIET, $SHOW_QUERY_ONLY;
	$bind_vars = Array();

	$sql = 'DELETE FROM'."\n";
	$sql .= '    '.SQ_TABLE_RUNNING_PREFIX.'internal_msg'."\n";
	$sql .= 'WHERE'."\n";
	$sql .= '    sent <= :sent_before'."\n";
	$bind_vars['sent_before'] = $period;

	$userids = Array(
				Array(
					'field_name'	=> 'userfrom',
					'value'			=> (string)$user_from,
				),
				Array(
					'field_name'	=> 'userto',
					'value'			=> (string)$user_to,
				),
			);

	foreach ($userids as $userid) {

		if (strlen(trim($userid['value'])) != 0) {
			if ($userid['value'] == 'all') {

				// All messages sent from/to users
				$sql .= '    AND '.$userid['field_name'].' <> '.MatrixDAL::quote('0')."\n";

			} else if (strpos($userid['value'], ':') !== FALSE) {

				// Multiple userids found
				$ids = explode(':', $userid['value']);
				if (count($ids) >= 1) {
					$sql .= '    AND (';
					foreach ($ids as $id) {
						if (strlen(trim($id)) == 0) continue;
						if (trim($id) == 'all') usage(TRUE);
						if (strpos($id, '*') !== FALSE && substr($id, -1) == '*') {
							$sql .= $userid['field_name'].' LIKE '.MatrixDAL::quote(substr($id, 0, -1).':%').' OR ';
						} else {
							$sql .= $userid['field_name'].' = '.MatrixDAL::quote($id).' OR ';
						}
					}
					$sql = substr($sql, 0, -4).')'."\n";
				}

			} else {

				// Single Userid found
				if (strpos($userid['value'], '*') !== FALSE && substr($userid['value'], -1) == '*') {
					$sql .= '    AND '.$userid['field_name'].' LIKE '.MatrixDAL::quote(substr($userid['value'], 0, -1).':%')."\n";
				} else {
					$sql .= '    AND '.$userid['field_name'].' = '.MatrixDAL::quote($userid['value'])."\n";
				}

			}
		}

	}//end foreach userids

	// Type of message
	if (!empty($msg_type)) {
		if (strpos($msg_type, '*') !== FALSE && substr($msg_type, -1) == '*') {
			$sql .= '    AND type LIKE :msg_type'."\n";
			$bind_vars['msg_type'] = substr($msg_type, 0, -1).'%';
		} else {
			$sql .= '    AND type = :msg_type'."\n";
			$bind_vars['msg_type'] = $msg_type;
		}
	}

	// Message Status
	if (!empty($msg_status)) {
		if (strpos($msg_status, ':') !== FALSE) {
			$tmp = explode(':', $msg_status);
			$sql .= '    and status IN (';
			foreach($tmp as $token) {
				$sql .= MatrixDAL::quote($token).', ';
			}
			$sql = substr($sql, 0, -2).")\n";
		} else {
			$sql .= '    AND status = :msg_status'."\n";
			$bind_vars['msg_status'] = $msg_status;
		}
	}

	if (!empty($assetids)) {
		$sql .= ' and assetid IN (';
		foreach ($assetids as $_id => $assetid) {
			$sql .= MatrixDAL::quote($assetid).', ';
		}
		$sql = substr($sql, 0, -2).")\n";
	}

	$query = MatrixDAL::preparePdoQuery($sql);
	foreach ($bind_vars as $bind_var => $bind_value) {
		MatrixDAL::bindValueToPdo($query, $bind_var, $bind_value);
	}
	MatrixDAL::execPdoQuery($query);
	$affected_rows = MatrixDAL::getDbType() == 'oci' ? oci_num_rows($query) : $query->rowCount();

	if (!$QUIET) {
		echo "\n".$affected_rows.' INTERNAL MESSAGES '.($SHOW_QUERY_ONLY ? 'CAN BE ' : '').'DELETED'."\n\n";
	}

	if ($SHOW_QUERY_ONLY) {
		echo str_repeat('*', 50)."\n";
		echo '* Expected SQL query to run'."\n";
		echo str_repeat('*', 50)."\n";
		echo $sql;
		echo str_repeat('*', 50)."\n";
	}

}//end purge_internal_message()


/**
* Prints the usage for this script and exits
*
* @param boolean	$rollback	If it's TRUE, it cancels the transaction before it exits
*
* @return void
* @access public
*/
function usage($rollback=FALSE)
{
	echo "\nUSAGE: remove_internal_message.php -s <system_root> -p <period> [-f userfrom] [-t userto] [-y msg type] [-u status] [-a assetid]... [--show-query-only] [--quiet]\n\n".
		"-p The period to remove internal messages before\n".
		"-f The userid that the message is sent from, e.g. all or 7, 229*, 229*:323*:7\n".
		"-t The userid that the message is sent to, e.g. all or 7, 229*, 229*:323*:7\n".
		"-y The type of internal message to remove, e.g. asset.linking.create, cron.*\n".
		"-u The status of internal message to remove, e.g. U(nread), R(ead), D(eleted) or multiple like U:R or R:D\n".
		"-a The assetid that message belongs to, e.g. 7, 12, 111:222. This option can be specified multiple times e.g. \"-a 7 -a 12\".\n".
		"--show-query-only The script only prints the query without running it.\n".
		"--quiet No output will be sent\n".
		"(For -p, the period is in the format nx where n is the number of units and x is one of:\n".
		" h - hours\t\n d - days\t\n w - weeks\t\n m - months\t\n y - years)\n\n".
		"(For -f and -t, the userid can take different formats which is one of:\n".
		" all\t\t- internal messages that are sent from/to other than system, i.e. not equal to 0\t\n".
		" single id\t- for example, 7 for public user or 221* for LDAP bridge(#221) users\t\n".
		" multiple ids\t- more than one of the above single ids, for example, 7:221*:328* for Public or LDAP bridge(#221) or IPB bridge(#328) users)\t\n\n".
		"Examples:\n\n".
		"1. Delete all the internal messages older than last 2 days\n".
		"   \$ php remove_internal_message.php -s SYSTEM_ROOT -p 2d\n\n".
		"2. Delete all the internal messages sent to LDAP Bridge(#221) and IPB Bridge(#328) users in last 2 days\n".
		"   \$ php remove_internal_message.php -s SYSTEM_ROOT -p 2d -t 221*:328*\n\n".
		"3. Delete all the internal messages sent from the system and Message type starts with asset in last 1 month\n".
		"   \$ php remove_internal_message.php -s SYSTEM_ROOT -p 1m -f 0 -y asset.*\n\n".
		"4. Delete all the internal messages sent to users have been deleted in last 3 days\n".
		"   \$ php remove_internal_message.php -s SYSTEM_ROOT -p 3d -t all -u D\n\n".
		"5. Delete all the internal messages belonging to the assets #7 and #12\n".
		"   \$ php remove_internal_message.php -s SYSTEM_ROOT -a 7 -a 12\n\n".
		"Inbox Message Examples:\n\n".
		"1. Delete all read and deleted inbox messages (sent and inbox) in last 3 days\n".
		"   \$ php remove_internal_message.php -s SYSTEM_ROOT -p 3d -y inbox.* -u R:D\n\n".
		"2. Delete all read inbox sent messages in last 3 days\n".
		"   \$ php remove_internal_message.php -s SYSTEM_ROOT -p 3d -y inbox.sent -u R\n\n".
		"3. Delete all deleted inbox messages (sent and inbox) sent to LDAP bridge(#211) users in last 3 days\n".
		"   \$ php remove_internal_message.php -s SYSTEM_ROOT -p 3d -y inbox.* -u D -t 221*\n\n";

	if ($rollback) {
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
	}

	exit();

}//end usage()


?>
