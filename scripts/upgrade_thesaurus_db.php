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
* $Id: upgrade_thesaurus_db.php,v 1.2 2005/12/09 05:32:16 arailean Exp $
*
*/

/**
* Upgrades thesaurus contents from 0.1 to 0.2
*
* @author  Elden McDonald
* @version $Revision: 1.2 $
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

$GLOBALS['SQ_SYSTEM']->am->includeAsset('thesaurus_term');


$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');


 //Add a column to the old table to mark a record as deleted.
// Add another column that marks a record as seen

$sql = 'ALTER TABLE ONLY sq_lex_thes_ent_rel
		 ADD deleted boolean';
	$result = $db->query($sql);
	assert_valid_db_result($result);
$sql = 'ALTER TABLE ONLY sq_lex_thes_ent_rel
		ADD depth integer';
	$result = $db->query($sql);
	assert_valid_db_result($result);

//Retrieve a list of the thesaurii
$thesaurii = $am->getTypeAssetids('thesaurus');

foreach ($thesaurii as $thesaurus_id) {
	$thesaurus = &$am->getAsset($thesaurus_id);
	$depth=1;
	$num_children=0;

	$term_id_map = Array();

	//Select all the absolute parent terms for this thesaurus as a starting point
	printName('Retrieving absolute parents for '.$thesaurus_id);
	$sql = 'SELECT name, id
			FROM sq_lex_thes_ent
			WHERE id NOT in (SELECT cld_ent_id
							FROM sq_lex_thes_ent_rel
							WHERE thes_id='.$db->quoteSmart($thesaurus_id).')
			AND thes_id = '.$db->quoteSmart($thesaurus_id);
	$result = $db->query($sql);
	assert_valid_db_result($result);
	printUpdateStatus('OK');
	echo '... Processing '.$result->numRows().' links at level '.$depth."\n";
	while (null !== ($row = $result->fetchRow())) {

		// link each absolute parent to the null term in the new tables
		$minor = $row['name'];
		$minor_id = $row['id'];
		//Use the thesaurus instance to add the term link from abs parent to null term

		$create_link['asset'] = &$thesaurus;
		$create_link['value'] = null;
		$term_asset = new Thesaurus_Term();
		$term_asset->setAttrValue('name', $minor);
		if(!$term_asset->create($create_link)) {
			trigger_error('Move term to new thesaurus: "'.$minor.'" #'.$minor_id, E_USER_FAILURE);
			return false;
		}

		$term_id_map[$minor_id] = $term_asset->id;

		//Now mark each relation in which this entity is a parent as depth 1.
		$sql ='
			UPDATE sq_lex_thes_ent_rel
			SET depth = '.$db->quoteSmart($depth).'
			WHERE pnt_ent_id = '.$minor_id.'
			AND thes_id = '.$db->quoteSmart($thesaurus_id);

		$update_result   = $db->query($sql);
		assert_valid_db_result($update_result);

		$num_children++;
		if (bcmod($num_children,1000)==0) {
			echo ' Processed '.$num_children."\n";
		}
	}
	echo ' Finished '.$num_children."\n";

	while ($num_children>0) {
		$num_children=0;
		//Traverse the tree level by level until we have processed all the descendents of the absolute parent nodes
		printName('Retrieving relations at depth '.($depth+1).' for thesaurus '.$thesaurus_id);

		$sql = '
			SELECT
				e1.name as major,
				e1.id as major_id,
				e2.name as minor,
				e2.id as minor_id,
				r.id as rel_id,
				r.name as relation,
				er.thes_id as thesid

			FROM
				sq_lex_thes_ent_rel er
				INNER JOIN sq_lex_thes_rel r ON er.rel_id = r.id
				INNER JOIN sq_lex_thes_ent e1 ON er.pnt_ent_id = e1.id
				INNER JOIN sq_lex_thes_ent e2 ON er.cld_ent_id = e2.id

			WHERE
				er.depth = '.$db->quoteSmart($depth).'
				AND
				er.thes_id = '.$db->quoteSmart($thesaurus_id).'
				AND
				(
					er.deleted IS FALSE
					OR
					er.deleted IS NULL
				)
			ORDER BY major
		';

		$result = $db->query($sql);
		assert_valid_db_result($result);
		printUpdateStatus('OK');
		$depth++;
		echo '... Processing '.$result->numRows().' links at level '.($depth)."\n";
		while (null !== ($row = $result->fetchRow())) {

			$minor = $row['minor'];
			$minor_id = $row['minor_id'];
			$major = $row['major'];
			$major_id = $row['major_id'];
			$relation = $row['relation'];
			$rel_id = $row['rel_id'];

			$major_asset = &$am->getAsset($term_id_map[$major_id]);
			//Use the thesaurus instance to add this term link

			$create_link['asset'] =& $major_asset;
			$create_link['value'] = $relation;

			$term_asset = new Thesaurus_Term();
			$term_asset->setAttrValue('name', $minor);
			if(!$term_asset->create($create_link)) {
				trigger_error('Move term to new thesaurus: "'.$minor.'" #'.$minor_id, E_USER_FAILURE);
				return false;
			}

			$term_id_map[$minor_id] = $term_asset->id;

			//Mark each relation in which the minor is a parent as depth++.
			$sql ='
				UPDATE sq_lex_thes_ent_rel SET depth = '.$db->quoteSmart($depth).'
				WHERE pnt_ent_id = '.$db->quoteSmart($minor_id).'
				AND thes_id = '.$db->quoteSmart($thesaurus_id).'
				AND depth IS NULL';
			$update_result = $db->query($sql);
			assert_valid_db_result($update_result);

			//Delete this node to avoid loops
			$sql ='
				UPDATE sq_lex_thes_ent_rel
				SET deleted = '.$db->quoteSmart('TRUE').'
				WHERE cld_ent_id = '.$minor_id.'
				AND rel_id = '.$db->quoteSmart($rel_id).'
				AND thes_id = '.$db->quoteSmart($thesaurus_id);
			$update_result   = $db->query($sql);
			assert_valid_db_result($update_result);
			$num_children++;

			if (bcmod($num_children,1000)==0) {
				echo ' Processed '.$num_children."\n";
			}
		}
		echo ' Finished '.$num_children."\n";

	}

	// By this point, we have moved all the term links that are descended from an absolute parent node
	// There may be orphan trees where all the parents are also children, creating a looped tree with no clear absolute parents
	// If they exist, we take an arbitrary term and link it to the null term in the new table
	// which allows us to traverse it like a normal tree

	while (_termLinksRemaining($thesaurus_id)) {

		printName('Retrieving orphan looped trees for thesaurus '.$thesaurus_id);
		$depth = 1;
		$sql = 'SELECT name, id
					FROM sq_lex_thes_ent
					WHERE id IN (SELECT distinct pnt_ent_id
									FROM sq_lex_thes_ent_rel
									WHERE thes_id='.$db->quoteSmart($thesaurus_id).'
									AND(deleted IS FALSE
										OR deleted IS NULL))
					AND thes_id = '.$db->quoteSmart($thesaurus_id);

		$result = $db->query($sql);
		assert_valid_db_result($result);
		printUpdateStatus('OK');

		if ($row = $result->fetchRow()) {
			// Link the first node selected from the looped orphan trees
			$minor = $row['name'];
			$minor_id = $row['id'];
			printStep('Linking '.$minor.' to the null term for thesaurus '.$thesaurus_id);

			$create_link['asset'] = &$thesaurus;
			$create_link['value'] = null;
			$term_asset = new Thesaurus_Term();
			$term_asset->setAttrValue('name', $minor);
			if(!$term_asset->create($create_link)) {
				trigger_error('Move term to new thesaurus: "'.$minor.'" #'.$minor_id, E_USER_FAILURE);
				return false;
			}

			$term_id_map[$minor_id] = $term_asset->id;

			//Now mark each relation in which this entity is a parent as depth 1
			$sql ='
				UPDATE sq_lex_thes_ent_rel SET depth = '.$db->quoteSmart($depth).'
				WHERE pnt_ent_id = '.$minor_id;

			$update_result   = $db->query($sql);
			assert_valid_db_result($update_result);
			printUpdateStatus('OK');

			$num_children++;
			// Process the descendants of this term by depth until we reach a depth with no children in it
			while ($num_children>0) {
				$num_children=0;

				printName('Retrieving relations at depth '.($depth+1).' for thesaurus '.$thesaurus_id);
				$sql = '
					SELECT
						e1.name as major,
						e1.id as major_id,
						e2.name as minor,
						e2.id as minor_id,
						r.id as rel_id,
						r.name as relation,
						er.thes_id as thesid
					FROM
						sq_lex_thes_ent_rel er
						INNER JOIN sq_lex_thes_rel r ON er.rel_id = r.id
						INNER JOIN sq_lex_thes_ent e1 ON er.pnt_ent_id = e1.id
						INNER JOIN sq_lex_thes_ent e2 ON er.cld_ent_id = e2.id
					WHERE
						er.depth = '.$db->quoteSmart($depth).'
						AND
						er.thes_id = '.$db->quoteSmart($thesaurus_id).'
						AND
						(
							er.deleted IS FALSE
							OR
							er.deleted IS NULL
						)
					ORDER BY major
				';
				$result = $db->query($sql);
				assert_valid_db_result($result);
				printUpdateStatus('OK');

				$depth++;

				echo '... Processing '.$result->numRows().' links at level '.($depth)."\n";
				while (null !== ($row = $result->fetchRow())) {
					$minor = $row['minor'];
					$minor_id = $row['minor_id'];
					$major = $row['major'];
					$major_id = $row['major_id'];
					$relation = $row['relation'];
					$rel_id = $row['rel_id'];
					//Use the thesaurus instance to add this term link

					$major_asset = &$am->getAsset($term_id_map[$major_id]);
					//Use the thesaurus instance to add this term link

					$create_link['asset'] =& $major_asset;
					$create_link['value'] = $relation;

					$term_asset = new Thesaurus_Term();
					$term_asset->setAttrValue('name', $minor);
					if(!$term_asset->create($create_link)) {
						trigger_error('Move term to new thesaurus: "'.$minor.'" #'.$minor_id, E_USER_FAILURE);
						return false;
					}

					$term_id_map[$minor_id] = $term_asset->id;

					//Mark each relation in which the minor is a parent as depth++.
					$sql ='
						UPDATE sq_lex_thes_ent_rel SET depth = '.$db->quoteSmart($depth).'
						WHERE pnt_ent_id = '.$db->quoteSmart($minor_id).'
						AND thes_id = '.$db->quoteSmart($thesaurus_id).'
						AND depth IS NULL';
					$update_result = $db->query($sql);
					assert_valid_db_result($update_result);

					//Delete this node to avoid loops
					$sql ='
						UPDATE sq_lex_thes_ent_rel
						SET deleted = '.$db->quoteSmart('TRUE').'
						WHERE cld_ent_id = '.$minor_id.'
						AND rel_id = '.$db->quoteSmart($rel_id).'
						AND thes_id = '.$db->quoteSmart($thesaurus_id);
					$update_result   = $db->query($sql);
					assert_valid_db_result($update_result);
					$num_children++;

					if (bcmod($num_children,1000)==0) {
						echo ' Processed '.$num_children."\n";
					}
				}
				echo ' Finished '.$num_children."\n";
			}
		}
	}
	$thesaurus->markContentsChanged();
}


/**
*
* Check if there are term links that have not been migrated
*
* @param int	$thesaurus_id	id of thesaurus to check
*
* @return boolean
* @access private
*/
function _termLinksRemaining($thesaurus_id)
{
	$db = &$GLOBALS['SQ_SYSTEM']->db;
	$sql = 'SELECT count(*)
			FROM sq_lex_thes_ent_rel
			WHERE thes_id = '.$db->quoteSmart($thesaurus_id).'
			AND (deleted IS FALSE OR deleted IS NULL)';
	$result = $db->getOne($sql);
	assert_valid_db_result($result);
	return($result);
}//end _termLinksRemaining()


$sql = 'DROP TABLE sq_lex_thes_ent, sq_lex_thes_rel, sq_lex_thes_ent_rel';
$result = $db->query($sql);
assert_valid_db_result($result);

$sql = 'DROP SEQUENCE sq_lex_thes_ent_id_seq, sq_lex_thes_rel_id_seq';
$result = $db->query($sql);
assert_valid_db_result($result);

$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

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
	echo "[ $status ]\n";
}//end printUpdateStatus()


?>
