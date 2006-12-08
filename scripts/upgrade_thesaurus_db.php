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
* $Id: upgrade_thesaurus_db.php,v 1.6 2006/12/08 04:01:42 arailean Exp $
*
*/

/**
* Upgrades thesaurus contents from 0.1 to 0.2
*
* @author  Andrei Railean
* @author  Elden McDonald
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
error_reporting(E_ALL);
if ((php_sapi_name() != 'cli')) {
	trigger_error("You can only run this script from the command line\n", E_USER_ERROR);
}

$SYSTEM_ROOT = (isset($_SERVER['argv'][1])) ? $_SERVER['argv'][1] : '';
if (empty($SYSTEM_ROOT) || !is_dir($SYSTEM_ROOT)) {
	trigger_error("You need to supply the path to the System Root as the first argument\n", E_USER_ERROR);
}

require_once $SYSTEM_ROOT.'/core/include/init.inc';


// ask for the root password for the system
echo 'Enter the root password for "'.SQ_CONF_SYSTEM_NAME.'": ';
$root_password = rtrim(fgets(STDIN, 4094));


// check that the correct root password was entered
$root_user =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
if (!$root_user->comparePassword($root_password)) {
	trigger_error("The root password entered was incorrect\n", E_USER_ERROR);
}

// log in as root
if (!$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user)) {
	trigger_error("Failed logging in as root user\n", E_USER_ERROR);
}

printUpdateStatus('Begin Thesaurus Upgrade');

$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

$db =& $GLOBALS['SQ_SYSTEM']->db;
$am =& $GLOBALS['SQ_SYSTEM']->am;

// drop 'sort_order' column because it won't be used anymore
$sql = '
	ALTER TABLE
		sq_thes_lnk
	DROP
		sort_order
';
$result = $db->query($sql);
assert_valid_db_result($result);

// add relid column
$sql = '
	ALTER TABLE
		sq_thes_lnk
	ADD
		relid varchar(15)
';
$result = $db->query($sql);
assert_valid_db_result($result);

// create an index on the new relid column
$sql = 'CREATE INDEX sq_thes_lnk_relid ON sq_thes_lnk(relid)';
$result = $db->query($sql);
assert_valid_db_result($result);

// delete NULL terms - no more BS
$sql = '
	DELETE FROM
		sq_thes_lnk
	WHERE
		major IS NULL;
';
$result = $db->query($sql);
assert_valid_db_result($result);



$thesaurii = $am->getTypeAssetids('thesaurus');

foreach ($thesaurii as $thesaurus_id) {
	printUpdateStatus('Upgrading Thesaurus #'.$thesaurus_id);
	$thesaurus =& $am->getAsset($thesaurus_id);

	$sql = '
		SELECT DISTINCT
			"relation"
		FROM
			sq_thes_lnk
		WHERE
			thesid = '.$db->quoteSmart($thesaurus_id)
	;

	$result = $db->getCol($sql);
	assert_valid_db_result($result);

	foreach ($result as $relation_name) {
		// thesaurus will automatically convert '' into NULL
		// when calling addRelation with '' and NULL you will get the same ID
		$new_relid = $thesaurus->addRelation($relation_name);

		$sql ='
			UPDATE
				sq_thes_lnk
			SET
				relid = '.$db->quoteSmart($new_relid).'
			WHERE
				relation = '.$db->quoteSmart($relation_name).'
				AND
				thesid = '.$db->quoteSmart($thesaurus_id);

		$update_result   = $db->query($sql);
		assert_valid_db_result($update_result);
		echo '.';
	}


	if (!$am->acquireLock($thesaurus->id, 'attributes', $thesaurus->id, TRUE, NULL)) {
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		printUpdateStatus('Failed acquiring the attribute lock');
		continue;
	}

	$abbreviation_attr =& $thesaurus->getAttribute('abbreviation_rel');
	$abbr_rel_name = $abbreviation_attr->value;
	if (empty($abbr_rel_name)) $abbr_rel_name = NULL;

	$abbr_rel_id = $thesaurus->getRelationIdByName($abbr_rel_name);
	$thesaurus->setAttrValue('abbreviation_rel', $abbr_rel_id);
	echo '.';

	$syn_attr =& $thesaurus->getAttribute('synonym_rel');
	$syn_rel_name = $syn_attr->value;
	if (empty($syn_rel_name)) $syn_rel_name = NULL;

	$syn_rel_id = $thesaurus->getRelationIdByName($syn_rel_name);
	$thesaurus->setAttrValue('synonym_rel', $syn_rel_id);
	echo '.';

	$thesaurus->markContentsChanged();

	if (!$thesaurus->saveAttributes()) {
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		printUpdateStatus('Failed Saving attributes');
		continue;
	}

	$am->releaseLock($thesaurus->id, 'attributes');

	echo ' DONE';

}//end foreach


// now that relations have been moved to a different table, drop the column
$sql = '
	ALTER TABLE ONLY
		sq_thes_lnk
	DROP
		relation
';
$result = $db->query($sql);
assert_valid_db_result($result);


// tree IDs are dropped. this step is the main reason for the upgrade.
$sql = 'DROP TABLE sq_thes_lnk_tree';
$result = $db->query($sql);
assert_valid_db_result($result);

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

printUpdateStatus('Complete');
echo "\n\n";
exit();
  ////////////////////////
 //  HELPER FUNCTIONS  //
////////////////////////


/**
* Output a step
*
* @param string	$name	name of the step
*
* @return boolean
* @access public
*/
function printName($name)
{
	printf ('%s%'.(85 - strlen($name)).'s', $name, '');

}//end printName()


/**
* Output an intended substep
*
* @param string	$name	name of the step
*
* @return boolean
* @access public
*/
function printStep($name)
{
	printf ('...  '.'%s%'.(80 - strlen($name)).'s', $name, '');

}//end printStep()


/**
* Output status of a step
*
* @param string	$status	status to output
*
* @return boolean
* @access public
*/
function printUpdateStatus($status)
{
	echo "\n[ $status ]";

}//end printUpdateStatus()


?>
