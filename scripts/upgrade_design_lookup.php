<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: upgrade_design_lookup.php,v 1.5.4.1 2006/09/27 23:53:00 gsherwood Exp $
*
*/

/**
* Upgrade the *_ast_lookup_design table to *_ast_lookup_value
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.5.4.1 $
* @package MySource_Matrix
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) trigger_error("You can only run this script from the command line\n", E_USER_ERROR);

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';

// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));

// check that the correct root password was entered
$root_user = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	trigger_error("The root password entered was incorrect\n", E_USER_ERROR);
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

$db = &$GLOBALS['SQ_SYSTEM']->db;
$am = &$GLOBALS['SQ_SYSTEM']->am;

// first check that the table we are going to rename to is not already there.
$tables = $db->getListOf('tables');
assert_valid_db_result($tables);
if (!in_array('sq_ast_lookup_value', $tables)) {
	trigger_error('You need to run install/step_02.php to install the new table required', E_USER_ERROR);
}

// if the old table doesn't exist, there isn't anything we can do.
if (!in_array('sq_ast_lookup_design', $tables)) {
	trigger_error('Cannot run upgrade script without old table. Have you run this script before ?', E_USER_ERROR);
}

unset($tables);

$count = $db->getOne('SELECT COUNT(*) FROM sq_ast_lookup_value');
assert_valid_db_result($count);

$count = (int) $count;

// if the new table has data, abort.
if ($count) {
	trigger_error('Refusing to run run upgrade script there is already data in the table sq_ast_lookup_value', E_USER_ERROR);
}

//--        FIX LOOKUP DESIGN TABLE        --//

// OK there was a problem with the old ast_lookup_design table.
// It had the designid as a field in the primary key when it shouldn't have. This means that there is the possibility of duplicate url and name combinations
// This shouldn't have ever happened int the live table for any system I have seen but is a definite problem for the rollback table

// v3_us_airtours=> SELECT sq_effective_from, url, name, count(*) FROM sq_rollback_asset_lookup_design GROUP BY sq_effective_from, url, name HAVING COUNT(*) > 1;
//   sq_effective_from  |                          url                          |          name           | count
// ---------------------+-------------------------------------------------------+-------------------------+-------
//  2004-05-27 12:39:19 | travelplanners.clients.squiz.co.uk/home               | system_design::frontend |     2
//  2004-05-27 12:39:19 | usairtours.clients.squiz.co.uk/home                   | system_design::frontend |     2

// So first things first it to try and fix this up for the live table

$sql = 'SELECT url, name
		FROM sq_ast_lookup_design
		GROUP BY url, name
		HAVING COUNT(*) > 1
		';

$design_lookups = $db->getAll($sql);
assert_valid_db_result($design_lookups);

foreach ($design_lookups as $row) {

	// OK, this is a bit crass, but there isn't really anything else we can do...

	// get the designid that we would use on the frontend for this url
	$used_design = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($row['url'], $row['name']);

	$sql = 'DELETE FROM sq_ast_lookup_design
			WHERE url  = '.$db->quote($row['url']).'
			  AND name = '.$db->quote($row['name']);
	// and if a design exists delete everything other than the design
	if (!empty($used_design)) {
		$sql .= '
			  AND designid != '.$db->quote($used_design['designid']);
	}

	$delete_result = $db->query($sql);
	assert_valid_db_result($delete_result);

}// end foreach

unset($design_lookups);


// Now try and fix the rollback table
$sql = 'SELECT sq_eff_from, url, name, COUNT(*) as count
		FROM sq_rb_ast_lookup_design
		GROUP BY sq_eff_from, url, name
		HAVING COUNT(*) > 1
		';

$design_lookups = $db->getAll($sql);
assert_valid_db_result($design_lookups);

foreach ($design_lookups as $row) {

	// OK, this is a bit crass, but there isn't really anything else we can do...

	// see if all these rollback entries have a null as the end date
	$sql = 'SELECT COUNT(*)
			FROM sq_rb_ast_lookup_design
			WHERE sq_eff_from  = '.$db->quote($row['sq_eff_from']).'
			  AND sq_eff_to IS NULL
			  AND url  = '.$db->quote($row['url']).'
			  AND name = '.$db->quote($row['name']);
	$num_no_end = $db->getOne($sql);

	$delete_sql = 'DELETE FROM sq_ast_lookup_design
					WHERE sq_eff_from  = '.$db->quote($row['sq_eff_from']).'
					  AND url  = '.$db->quote($row['url']).'
					  AND name = '.$db->quote($row['name']);

	// right we have all of then without an end....which means that these are current,
	// so delete all the entries that don't match the current one
	if ((int) $num_no_end == (int) $row['count']) {
		// get the designid that we would use on the frontend for this url
		$used_design = $GLOBALS['SQ_SYSTEM']->am->getDesignFromURL($row['url'], $row['name']);

	// must be an old entry, just take a guess....
	} else {

		// get the designid that we would use on the frontend for this url
		// NOTE: having to use direct sql because we can't change in and out of rollback view in a single script execution
		$sql = 'SELECT ld.designid, a.type_code
				FROM sq_rb_ast_lookup l
						INNER JOIN sq_rb_ast_lookup_design ld ON l.url = ld.url
						INNER JOIN sq_rb_ast a ON ld.designid = a.assetid
				WHERE ld.url = '.$db->quote($row['url']).'
				  AND ld.name = '.$db->quote($row['name']);
		foreach (Array('ld.', 'l.', 'a.') as $table_alias) {
			$sql .= '
				  AND '.$table_alias.'sq_eff_from <= '.$db->quote($row['sq_eff_from']).'
				  AND ('.$table_alias.'sq_eff_to IS NULL
						   OR '.$table_alias.'sq_eff_to > '.$db->quote($row['sq_eff_from']).')';
		}

		$used_design = $db->getRow($sql);
		assert_valid_db_result($used_design);

	}


	$sql = 'DELETE FROM sq_rb_ast_lookup_design
			WHERE sq_eff_from  = '.$db->quote($row['sq_eff_from']).'
			  AND url  = '.$db->quote($row['url']).'
			  AND name = '.$db->quote($row['name']);
	if (!empty($used_design)) {
		$sql .= '
			  AND designid != '.$db->quote($used_design['designid']);
	}

	$delete_result = $db->query($sql);
	assert_valid_db_result($delete_result);

}// end foreach

unset($design_lookups);


//--        COPY DATA        --//


require_once SQ_FUDGE_PATH.'/db_extras/db_extras.inc';
foreach (Array('sq_', 'sq_rb_') as $prefix) {

	$extra = ($prefix == 'sq_rb_') ? ', sq_eff_from, sq_eff_to' : '';

	printName('Copying from '.$prefix.'ast_lookup_design to '.$prefix.'ast_lookup_value');

	$insert_sql = 'INSERT INTO '.$prefix.'ast_lookup_value (url, name, value, inhd'.$extra.')';
	$select_sql = 'SELECT url, '.$db->quote('design::').' || replace(name, '.$db->quote('_design::').', '.$db->quote('::').'), designid, '.$db->quote('0').$extra.' FROM '.$prefix.'ast_lookup_design';
	$result = db_extras_insert_select($db, $insert_sql, $select_sql);
	assert_valid_db_result($result);

	printUpdateStatus('OK');

}

//--        DROP LOOKUP DESIGN TABLE        --//


printName('Dropping ast_lookup_design');
$result = $db->query('DROP TABLE sq_ast_lookup_design');
assert_valid_db_result($result);
$result = $db->query('DROP TABLE sq_rb_ast_lookup_design');
assert_valid_db_result($result);
printUpdateStatus('OK');


//--        UPDATE VALUES FOR DESIGN LINKS        --//

foreach (Array('sq_', 'sq_rb_') as $prefix) {

	$extra = ($prefix == 'sq_rb_') ? ', sq_eff_from, sq_eff_to' : '';

	printName('Updating '.$prefix.'ast_lnk');

	$sql = 'UPDATE '.$prefix.'ast_lnk
			SET value = '.$db->quote('design::').' || replace(value, '.$db->quote('_design::').', '.$db->quote('::').')
			WHERE link_type = '.$db->quote(SQ_LINK_NOTICE).'
			  AND (value LIKE '.$db->quote('system_design::%').' OR value LIKE '.$db->quote('user_design::%').')';

	$result = $result = $db->query($sql);
	assert_valid_db_result($result);

	printUpdateStatus('OK');

}


?>
If everything looks successful, then run :

        php <?php echo $SYSTEM_ROOT; ?>/scripts/system_update_lookups.php <?php echo $SYSTEM_ROOT; ?>

Thanks
<?php


exit();


//--        HELPER FUNCTIONS        --//

function printName($name)
{
	printf ('%s%'.(60 - strlen($name)).'s', $name, '');

}//end printName()


function printUpdateStatus($status)
{
	echo "[ $status ]\n";

}//end printUpdateStatus()


?>
