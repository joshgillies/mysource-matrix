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
* $Id: thesaurus.inc,v 1.4 2005/12/05 00:33:40 arailean Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

register_implementation('thesaurus', 'bridge');

/**
* Thesaurus
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Elden McDonald <emcdonald@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Thesaurus extends Asset
{

	var $REL_ABBREVIATION = 'Abbreviation';


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Thesaurus($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Delete Asset
	*
	* @param boolean	$release_lock	true, to release lock
	*
	* @return boolean
	* @access public
	*/
	function delete($release_lock)
	{
		if (parent::delete($release_lock)) {
			$this->_deleteStoredContents();
			return true;
		}

		return false;

	}//end delete()


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Thesaurii create their data directory when created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;
		return create_directory($this->data_path);

	}//end _createAdditional()


//--        ABBREVIATIONS        --//


	/**
	* Get an array of abbrevations
	*
	* the result is an array of format ("A" => "Australia")
	* abbreviation => expansion
	* NOTICE: An expansion can have only one abbreviation
	*
	* @return array
	* @access public
	*/
	function getAbbreviations()
	{
		//TODO: add abbreviation support

		$result = Array();

		return $result;

	}//end getAbbreviations()


//--        CORE THESAURUS RELATIONAL ENGINE        --//


	/**
	* Get minor terms
	*
	* Returns an array of terms which have a given term as their parent
	*
	* @param int	$termid	Term
	*
	* @return array
	* @access public
	*/
	function getChildTerms($termid)
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				linkid, term, termid, relation, sort_order
			FROM
				sq_thes_term term
				INNER JOIN sq_thes_lnk link ON term.termid = link.minor
			WHERE
				term.thesid = '.$db->quoteSmart($this->id).'
				AND
				link.thesid = '.$db->quoteSmart($this->id).'
				AND
				major '.(is_null($termid)?'ISNULL':'= '.$db->quoteSmart($termid)).'
			ORDER BY
				sort_order
		';

		$result = $db->getAll($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getChildTerms()


	/**
	* Count child terms
	*
	* Get a count of how many terms are linked directly under a given term
	*
	* @param int	$termid	Term
	*
	* @return array
	* @access public
	*/
	function countChildTerms($termid)
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				count(linkid)
			FROM
				sq_thes_lnk
			WHERE
				thesid = '.$db->quoteSmart($this->id).'
				AND
				major '.(is_null($termid)?'ISNULL':'= '.$db->quoteSmart($termid)).'
		';

		$result = $db->getOne($sql);
		assert_valid_db_result($result);

		return $result;

	}//end countChildTerms()


	/**
	* Count all term links
	*
	* Get a count of how many terms are linked directly under a given term
	*
	* @return array
	* @access public
	*/
	function countTermLinks()
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				count(*)
			FROM
				sq_thes_lnk
			WHERE
				thesid = '.$db->quoteSmart($this->id).'
		';

		$result = $db->getOne($sql);
		assert_valid_db_result($result);

		return $result;

	}//end countTermLinks()


	/**
	* Get Child Relations for Term
	*
	* Get the relations a given term is a parent in
	*
	* @param string	$termid	term
	*
	* @return array
	* @access public
	*/
	function getChildRelationsForTerm($termid=null)
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql ='
			SELECT DISTINCT
				relation
			FROM
				sq_thes_lnk
			WHERE
				thesid = '.$db->quoteSmart($this->id).'
				AND
				major '.(is_null($term)?'ISNULL':'= '.$db->quoteSmart($termid)).'
		';

		$result = $db->getAll($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getChildRelationsForTerm()


	/**
	* Get Parent Relations for Term
	*
	* Get the relations a given term is a child in
	*
	* @param string	$termid	term
	*
	* @return array
	* @access public
	*/
	function getParentRelationsForTerm($termid=null)
	{

		if (!isset($term)) return null;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql ='
			SELECT DISTINCT
				relation
			FROM
				sq_thes_lnk
			WHERE
				thesid = '.$db->quoteSmart($this->id).'
				AND
				minor '.(is_null($term)?'ISNULL':'= '.$db->quoteSmart($termid)).'
		';

		$result = $db->getAll($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getParentRelationsForTerm()


	/**
	* Get parent terms
	*
	* Return terms which are immediate parents of a given term
	*
	* @param int	$termid	Term
	*
	* @return array
	* @access public
	*/
	function getParentTerms($termid)
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				linkid, term, termid, relation
			FROM
				sq_thes_term term
				INNER JOIN sq_thes_lnk link ON term.termid = link.minor
			WHERE
				term.thesid = '.$db->quoteSmart($this->id).'
				AND
				link.thesid = '.$db->quoteSmart($this->id).'
				AND
				minor = '.$db->quoteSmart($termid).'
			ORDER BY
				sort_order
		';

		$result = $db->getAll($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getParentTerms()


	/**
	* Count parent terms
	*
	* @param int	$termid	Term
	*
	* @return array
	* @access public
	*/
	function countParentTerms($termid)
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				count(linkid)
			FROM
				sq_thes_lnk
			WHERE
				thesid = '.$db->quoteSmart($this->id).'
				AND
				minor = '.$db->quoteSmart($termid).'
		';

		$result = $db->getOne($sql);

		assert_valid_db_result($result);
		return $result;

	}//end countParentTerms()


	/**
	* Get All Relations
	*
	* Returns array of all relations known (used or not)
	*
	* @return array
	* @access public
	*/
	function getAllRelations()
	{

		return $this->attr('relations');

	}//end getAllRelations()


	/**
	* Add term to the thesaurus
	*
	* If the term being added already exists, it will be linked
	* Returns a link ID on success, NULL on failure
	*
	* @param string	$term			Term
	* @param string	$parent			Parent - if null, we're adding to the top level
	* @param string	$relation		Relation
	* @param string	$sort_order		Asset map sort order index
	* @param int	$manual_linkid	Link ID to use instead of the autogenerated one
	*
	* @return int
	* @access public
	*/
	function addTerm($term, $parent=null, $relation=null, $sort_order=-1, $manual_linkid=null)
	{

		if (!isset($term) || $term == '') {
			return null;
		}

		if ($relation == '') $relation = null;

		// check if parent exists
		// if parent does not exist - bail
		if ($parent == '') $parent = null;
		$parentid = null;

		if (!empty($parent)) {
			$parentid = $this->getTermIdByName($parent);
			if (is_null($parentid)) return null;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// determine what to use for a linkid
		if (!is_null($manual_linkid)) {
			$check_link = $this->getTermLinkById($manual_linkid);
			if (!empty($check_link)) {
				trigger_localised_error('CORE0238',E_USER_WARNING);
				return null;
			}
			$linkid = $manual_linkid;
		} else {
			$linkid = $db->nextId('sq_thes_lnk');
			assert_valid_db_result($linkid);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');


		// if term does not exist, add it
		$termid = $this->getTermIdByName($term);
		if (!empty($termid)) {
			// term exists. quickly check if the link exists to save us the headache of adding it
			$link_info = $this->getTermLinkByTermids($termid, $parentid, $relation);
			if (!is_null($link_info)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				return null;
			}
		} else {
			// ok, let's add the term
			$termid = $db->nextId('sq_thes_term');
			assert_valid_db_result($termid);

			$sql = '
				INSERT INTO sq_thes_term
					(
						termid,
						term,
						thesid
					)
				VALUES
				(
					'.$termid.',
					'.$db->quoteSmart($term).',
					'.$db->quoteSmart($this->id).'
				)
			';
			$result = $db->query($sql);
			assert_valid_db_result($result);
		}


		$max_sort_order = $this->getMaxSortOrder($parentid);
		if (is_null($max_sort_order)) {
			$sort_order = 0;
		} else if (($sort_order > $max_sort_order) || ($sort_order < 0)) {
			$sort_order = $max_sort_order + 1;
		} else {
			// inserting somewhere in the middle
			// update all other sort orders before inserting this one
			// we're in a transaction, so if something goes wrong later, this will be rolled back
			$sql = '
				UPDATE
					sq_thes_lnk
				SET
					sort_order = sort_order + 1
				WHERE
					sort_order >= '.$db->quoteSmart($sort_order).'
					AND
					major '.(is_null($parentid)?'ISNULL':'= '.$db->quoteSmart($parentid)).'
					AND
					thesid = '.$db->quoteSmart($this->id).'
			';

			$result = $db->query($sql);
			assert_valid_db_result($result);
		}


		// inserting a term is the easiest thing ever
		$sql = '
			INSERT INTO sq_thes_lnk
				(
					linkid,
					major,
					minor,
					relation,
					sort_order,
					thesid
				)
			VALUES
				(
					'.$linkid.',
					'.$db->quoteSmart($parentid).',
					'.$db->quoteSmart($termid).',
					'.$db->quoteSmart($relation).',
					'.$db->quoteSmart($sort_order).',
					'.$db->quoteSmart($this->id).'
				)
		';

		$result = $db->query($sql);
		assert_valid_db_result($result);


		//// The Fun Part: Link Tree

		// This code is similar to the one in the asset manager, with a few exceptions
		// the major difference is that loops are allowed, i.e. a term can be its own grand-parent, etc
		// this requirement makes the tree operations more complicated than those of the asset manager

		// Thesaurus Tree Ids definitions:
		// -- Node - one level of the link tree that has tree ids pointing to it (everything is a node)
		// -- Incoming tree id - a tree id that a node has to its parent
		// -- Outgoing tree id - a tree id that a node has to its child

		// Thesaurus Tree Ids conditions:
		// - Every link has at least one tree id
		// - Every node has as many outgoing as incoming treeids
		//		MINUS the number of those incoming, which have another incoming as their subset
		// - A node cannot have an outgoing tree id that has another outgoing treeid as its subset (loop prevention)

		// Get one of the treeids that a parent term has
		if (is_null($parent)) {
			$parents_treeid = '';
		} else {
			$sql = '
				SELECT
					parent_tree.treeid
				FROM
					sq_thes_lnk_tree parent_tree
					INNER JOIN sq_thes_lnk parent_lnk ON parent_tree.linkid = parent_lnk.linkid,
					sq_thes_lnk_tree test_tree
					INNER JOIN sq_thes_lnk test_lnk ON test_tree.linkid = test_lnk.linkid
				WHERE
					parent_tree.treeid LIKE (test_tree.treeid || '.$db->quoteSmart('%').')

					AND
					parent_lnk.minor	= '.$db->quoteSmart($parentid).'
					AND
					test_lnk.minor		= '.$db->quoteSmart($parentid).'

					AND
					test_lnk.thesid		= '.$db->quoteSmart($this->id).'
					AND
					parent_lnk.thesid	= '.$db->quoteSmart($this->id).'
					AND
					parent_tree.thesid	= '.$db->quoteSmart($this->id).'
					AND
					test_tree.thesid	= '.$db->quoteSmart($this->id).'

				GROUP BY
					parent_tree.treeid
				HAVING
					count(parent_tree.treeid) = 1
			';

			$sql = $db->modifyLimitQuery($sql, 0, 1);

			$parents_treeid = $db->getOne($sql);
			assert_valid_db_result($parents_treeid);
		}
		$parents_treeid = (string) $parents_treeid;


		// when links are deleted their entries in the tree table are set to have a linkid of zero
		// (see deleteTermLink()) so we can attempt to find unused treeids that are forming gaps
		$sql = '
			SELECT
				treeid
			FROM
				sq_thes_lnk_tree
			WHERE
				treeid LIKE '.$db->quote($parents_treeid.'_%').'
				AND
				LENGTH(treeid) = '.(strlen($parents_treeid) + SQ_CONF_ASSET_TREE_SIZE).'
				AND
				linkid = 0
				AND
				thesid = '.$db->quoteSmart($this->id).'
		';

		$sql = $db->modifyLimitQuery($sql, 0, 1);
		$free_childid = $db->getOne($sql);
		assert_valid_db_result($free_childid);

		if (is_null($free_childid)) {
			// no free children, generate one from the max value (note that
			// we have to do this by a LIMIT query as the binary string fields
			// do not allow the MAX() aggregate to be run over it)
			$sql = '
				SELECT
					treeid
				FROM
					sq_thes_lnk_tree
				WHERE
					treeid LIKE '.$db->quote($parents_treeid.'_%').'
					AND
					LENGTH(treeid) = '.(strlen($parents_treeid) + SQ_CONF_ASSET_TREE_SIZE).'
					AND
					thesid = '.$db->quoteSmart($this->id).'
				ORDER BY
					treeid DESC
			';

			$free_childid = $db->getOne($db->modifyLimitQuery($sql, 0, 1));
			assert_valid_db_result($free_childid);

			if (is_null($free_childid)) {
				// no max, we must be the first
				if (($free_childid = asset_link_treeid_convert(0, true)) === false) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}
			} else {
				// we found a max so increment it

				// get only the child part
				$free_childid = substr($free_childid, -1 * SQ_CONF_ASSET_TREE_SIZE);

				if (($child_num = asset_link_treeid_convert($free_childid, false)) === false) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}
				if (($free_childid = asset_link_treeid_convert($child_num + 1, true)) === false) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return 0;
				}
			}//end if

		} else {
			// there was a free entry

			// get only the child part
			$free_childid = substr($free_childid, -1 * SQ_CONF_ASSET_TREE_SIZE);

			// remove the current zeroed entries in the tree table,
			// as we will be inserting over the top of them

			if (is_null($parent)) {
				$sql = '
					DELETE FROM
						sq_thes_lnk_tree
					WHERE
						treeid = '.$db->quoteSmart($free_childid).'
						AND
						thesid = '.$db->quoteSmart($this->id).'
				';
			} else {
				$sql = '
					DELETE FROM
						sq_thes_lnk_tree
					WHERE
						treeid IN
						(
							SELECT
								tree.treeid || '.$db->quoteSmart($free_childid).'
							FROM
								sq_thes_lnk_tree tree
								INNER JOIN sq_thes_lnk link ON tree.linkid = term.linkid
							WHERE
								link.minor	= '.$db->quoteSmart($parentid).'
								AND
								tree.thesid	= '.$db->quoteSmart($this->id).'
								AND
								link.thesid	= '.$db->quoteSmart($this->id).'
						)
						AND
						thesid = '.$db->quoteSmart($this->id).'
				';
			}

			$result = $db->query($sql);
			assert_valid_db_result($result);

		}//end if


		// if we don't have any existing tree entries then
		// we are the root term, so do a simple insert
		if ($parents_treeid == '') {
			$sql = '
				INSERT INTO
					sq_thes_lnk_tree
					(
						treeid,
						linkid,
						thesid
					)
				VALUES
					(
						'.$db->quoteSmart($free_childid).',
						'.$db->quoteSmart($linkid).',
						'.$db->quoteSmart($this->id).'
					)
			';

			$result = $db->query($sql);
			assert_valid_db_result($result);

		} else {
			// we have existing tree entries, do a insert..select to create entries for them all

			$sql = '
				INSERT INTO
					sq_thes_lnk_tree
					(
						treeid,
						linkid,
						thesid
					)
				SELECT
					(parent_tree.treeid || '.$db->quoteSmart($free_childid).') as treeid,
					'.$db->quoteSmart($linkid).' as linkid,
					'.$db->quoteSmart($this->id).' as thesid
				FROM
					sq_thes_lnk_tree parent_tree
					INNER JOIN sq_thes_lnk parent_link ON parent_tree.linkid = parent_link.linkid,
					sq_thes_lnk_tree test_tree
					INNER JOIN sq_thes_lnk test_link ON test_tree.linkid = test_link.linkid
				WHERE
					parent_tree.treeid LIKE (test_tree.treeid || '.$db->quoteSmart('%').')

					AND
					parent_link.minor	= '.$db->quoteSmart($parentid).'
					AND
					test_link.minor		= '.$db->quoteSmart($parentid).'

					AND
					test_link.thesid	= '.$db->quoteSmart($this->id).'
					AND
					parent_link.thesid	= '.$db->quoteSmart($this->id).'
					AND
					parent_tree.thesid	= '.$db->quoteSmart($this->id).'
					AND
					test_tree.thesid	= '.$db->quoteSmart($this->id).'

				GROUP BY
					parent_tree.treeid
				HAVING
					count(parent_tree.treeid) = 1
			';

			$result = $db->query($sql);
			assert_valid_db_result($result);

		}//end if


		// select the incoming tree ids that were assigned to this link id
		$sql = '
			SELECT
				treeid
			FROM
				sq_thes_lnk_tree
			WHERE
				linkid = '.$db->quoteSmart($linkid).'
				AND
				thesid = '.$db->quoteSmart($this->id).'
			ORDER BY
				treeid
		';
		$new_treeids = $db->getCol($sql);
		assert_valid_db_result($new_treeids);


		// now we propagate all new incoming tree ids to each outgoing tree branch
		// we cannot do this in one query because we have to check for loops and prune some tree branches
		// this section makes sure the above conditions for the link tree always hold
		foreach ($new_treeids as $new_treeid) {

			$sql = '
				INSERT INTO
					sq_thes_lnk_tree
					(
						treeid,
						linkid,
						thesid
					)
				SELECT DISTINCT
					'.$db->quoteSmart($new_treeid).' || SUBSTR(prop_tree.treeid, LENGTH(tree.treeid) +1 ),
					prop_tree.linkid,
					'.$db->quoteSmart($this->id).'
				FROM
					sq_thes_lnk_tree tree
					INNER JOIN sq_thes_lnk link ON tree.linkid = link.linkid,
					sq_thes_lnk_tree prop_tree

				WHERE
				-- Get the tree we are about to propagate to and siblings on that tree
					link.minor = '.$db->quoteSmart($termid).'
					AND
					tree.linkid <> '.$db->quoteSmart($linkid).'
					AND
					prop_tree.treeid LIKE (tree.treeid || '.$db->quoteSmart('_%').')
					AND
					prop_tree.treeid NOT IN
					(
						SELECT DISTINCT
							cut_tree.treeid

						FROM
							sq_thes_lnk_tree tree
							INNER JOIN sq_thes_lnk link ON tree.linkid = link.linkid,

							sq_thes_lnk_tree prop_tree
							INNER JOIN sq_thes_lnk_tree sibling_tree ON prop_tree.linkid = sibling_tree.linkid,

							sq_thes_lnk_tree cut_tree

						WHERE
							-- Get the tree we are about to propagate to and their siblings
							link.minor = '.$db->quoteSmart($termid).'
							AND
							tree.linkid <> '.$db->quoteSmart($linkid).'
							AND
							prop_tree.treeid LIKE (tree.treeid || '.$db->quoteSmart('_%').')

							-- Get the copy of the propagate tree we are about to cut
							AND
							cut_tree.treeid LIKE (tree.treeid || '.$db->quoteSmart('_%').')

							-- Find a prune tree in one of the siblings of the propagate tree
							-- This is a treeid that is a subset of the treeid that we are trying to propagate
							AND
							'.$db->quoteSmart($new_treeid).' LIKE (SUBSTR(sibling_tree.treeid, 1, LENGTH(sibling_tree.treeid) - '.$db->quoteSmart(SQ_CONF_ASSET_TREE_SIZE).') || '.$db->quoteSmart('%').')

							-- Everything thats below the parent of the propagate tree that has a sibling which is a subset
							-- of the new tree is to be cut out of propagation
							AND
							cut_tree.treeid LIKE (SUBSTR(prop_tree.treeid, 1, LENGTH(prop_tree.treeid) - '.$db->quoteSmart(SQ_CONF_ASSET_TREE_SIZE).') || '.$db->quoteSmart('_%').')

							AND
							tree.thesid 		= '.$db->quoteSmart($this->id).'
							AND
							link.thesid 		= '.$db->quoteSmart($this->id).'
							AND
							prop_tree.thesid 	= '.$db->quoteSmart($this->id).'
							AND
							sibling_tree.thesid	= '.$db->quoteSmart($this->id).'
							AND
							cut_tree.thesid 	= '.$db->quoteSmart($this->id).'
					)
					AND
					tree.thesid 		= '.$db->quoteSmart($this->id).'
					AND
					link.thesid 		= '.$db->quoteSmart($this->id).'
					AND
					prop_tree.thesid 	= '.$db->quoteSmart($this->id).'
			';
			$result = $db->query($sql);
			assert_valid_db_result($result);
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end addTerm()


	/**
	* Update term name and relation
	*
	* Returns true on success, false on failure
	*
	* @param int	$termid	link id to update
	* @param string	$name	the name of term
	*
	* @return boolean
	* @access public
	*/
	function updateTerm($termid, $name)
	{

		$term = $this->getTermById($termid);

		// if nothing has changed, no need to do any work
		if ($term['term'] == $name) return true;


		$db = &$GLOBALS['SQ_SYSTEM']->db;


		// if the new name is being used by another term, do not update
		// updating the name if it is used by another term is hard as we don't know what to do with the children
		// do we merge this term's children with the others' or do we only use our children or theirs?
		if ($this->isTerm($name)) {
			trigger_localised_error('CORE0239', E_USER_WARNING);
			return false;
		}


		// if the term has only one parent and the name is not used anywhere else - it is safe to update it
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		// update the term itself
		$sql = '
			UPDATE
				sq_thes_term
			SET
				term = '.$db->quoteSmart($name).'
			WHERE
				termid = '.$db->quoteSmart($termid).'
				AND
				thesid = '.$db->quoteSmart($this->id).'
		';

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end updateTerm()


	/**
	* Renames all instances of $relation to $new_relation
	*
	* Returns true on success, false on failure
	*
	* @param string	$relation		relation to rename
	* @param string	$new_relation	the desired new relation
	*
	* @return boolean
	* @access public
	*/
	function renameRelation($relation, $new_relation)
	{
		// Ensure the new name is not already in use

		if (array_search($new_relation, $this->getActiveRelations())!== FALSE) {
			//can't rename an existing term
			$this->_tmp['error'][] = translate('thesaurus_new_relation_in_use');
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// Update relations
		$sql = '
			UPDATE
				sq_thes_lnk
			SET
				relation = '.$db->quoteSmart($new_relation).'
			WHERE
				relation = '.$db->quoteSmart($relation).'
				AND
				thesid = '.$db->quoteSmart($this->id).'
		';
		$result = $db->query($sql);
		assert_valid_db_result($result);

		$this->markContentsChanged();
		return;

	}//end renameRelation()


	/**
	* Get term by its (link) id
	*
	* Gets the term that is linked as a minor by a particular link id
	*
	* @param int	$termid	link id of the term
	*
	* @return void
	* @access public
	*/
	function getTermById($termid)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				term, termid
			FROM
				sq_thes_term
			WHERE
				termid = '.$db->quoteSmart($termid).'
				AND
				thesid = '.$db->quoteSmart($this->id).'
		';
		$result = $db->getRow($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getTermById()


	/**
	* Get term by its link id
	*
	* Gets the term that is linked as a minor by a particular link id
	*
	* @param int	$linkid	link id of the term
	*
	* @return void
	* @access public
	*/
	function getTermByLinkId($linkid)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				term, termid
			FROM
				sq_thes_term term
				INNER JOIN sq_thes_lnk link ON  term.termid = link.minor
			WHERE
				link.linkid = '.$db->quoteSmart($linkid).'
				AND
				link.thesid = '.$db->quoteSmart($this->id).'
				AND
				term.thesid = '.$db->quoteSmart($this->id).'
		';
		$result = $db->getRow($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getTermByLinkId()


	/**
	* Get term link by its id
	*
	* Gets the term that is linked as a minor by a particular link id
	*
	* @param int	$linkid	link id of the term
	*
	* @return void
	* @access public
	*/
	function getTermLinkById($linkid)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				minor, major, relation, linkid, sort_order
			FROM
				sq_thes_lnk
			WHERE
				linkid = '.$db->quoteSmart($linkid).'
				AND
				thesid = '.$db->quoteSmart($this->id).'
		';
		$result = $db->getRow($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getTermLinkById()


	/**
	* Get term link id
	*
	* Given term information, returns a link id corresponding to that term
	*
	* @param string	$name		the name of the child in this link
	* @param string	$parent		the parent in this link
	* @param string	$relation	the relation of the link
	*
	* @return void
	* @access public
	*/
	function getTermLinkid($name, $parent=null, $relation=null)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				linkid
			FROM
				sq_thes_lnk
			WHERE
				minor = '.$db->quoteSmart($name).'
				AND
				thesid = '.$db->quoteSmart($this->id).'
				AND
				major '.(is_null($parent)?'ISNULL ':('= '.$db->quoteSmart($parent))).'
				AND
				relation '.(is_null($relation)?'ISNULL ':('= '.$db->quoteSmart($relation))).'
		';

		$result = $db->getOne($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getTermLinkid()


	/**
	* Get link by child and parent term ids
	*
	* @param string	$termid		the id of the child in this link
	* @param string	$parentid	the id of the parent in this link
	* @param string	$relation	the relation of the link
	*
	* @return void
	* @access public
	*/
	function getTermLinkByTermids($termid, $parentid=null, $relation=null)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				linkid,
				relation,
				sort_order,
				major,
				minor
			FROM
				sq_thes_lnk
			WHERE
				minor = '.$db->quoteSmart($termid).'
				AND
				major '.(is_null($parentid)?'ISNULL ':('= '.$db->quoteSmart($parentid))).'
				AND
				relation '.(is_null($relation)?'ISNULL ':('= '.$db->quoteSmart($relation))).'
				AND
				thesid = '.$db->quoteSmart($this->id).'
		';

		$result = $db->getRow($sql, DB_FETCHMODE_ASSOC);

		assert_valid_db_result($result);
		return $result;

	}//end getTermLinkByTermids()


	/**
	* Returns the lowest link id that has the supplied name as the minor
	*
	* @param string	$name	The name of the sought term
	*
	* @return int
	* @access public
	*/
	function getTermIdByName($name)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				termid
			FROM
				sq_thes_term
			WHERE
				term = '.$db->quoteSmart($name).'
				AND
				thesid = '.$db->quoteSmart($this->id).'
		';

		$result = $db->getOne($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getTermIdByName()


	/**
	* Check whether supplied string is a term in this thesaurus
	*
	* @param string	$term	Term
	*
	* @return boolean
	* @access public
	*/
	function isTerm($term=null)
	{
		if (is_null($term)) return false;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				termid
			FROM
				sq_thes_term
			WHERE
				term = '.$db->quoteSmart($term).'
				AND
				thesid = '.$db->quoteSmart($this->id).'
		';
		$result = $db->getOne($sql);

		assert_valid_db_result($result);

		return !empty($result);

	}//end isTerm()


	/**
	* Returns a list of all the relations in a thesaurus
	*
	* @return array
	* @access public
	*/
	function getActiveRelations()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT DISTINCT
				relation
			FROM
				sq_thes_lnk
			WHERE
				thesid = '.$db->quoteSmart($this->id).'
		';
		$result = $db->getCol($sql);

		assert_valid_db_result($result);

		return $result;

	}//end getActiveRelations()


	/**
	* Erases the contents of a thesaurus
	*
	* @return boolean
	* @access public
	*/
	function erase()
	{

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$this->setAttrValue('relations', Array());

		if ($this->_deleteStoredContents() && $this->saveAttributes()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return true;
		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

	}//end erase()


	/**
	* Deletes the contents of the database for this thesaurus
	*
	* @return boolean
	* @access private
	*/
	function _deleteStoredContents()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			DELETE FROM sq_thes_term WHERE thesid = '.$db->quoteSmart($this->id).';
			DELETE FROM sq_thes_lnk WHERE thesid = '.$db->quoteSmart($this->id).';
			DELETE FROM sq_thes_lnk_tree WHERE thesid = '.$db->quoteSmart($this->id).';
		';

		$result   = $db->query($sql);
		assert_valid_db_result($result);

		$this->markContentsChanged();
		return true;

	}//end _deleteStoredContents()


	/**
	* Get the path to the imported xml file
	*
	* @return string
	* @access public
	*/
	function getXmlFilePath()
	{
		return $this->data_path.'/content.xml';

	}//end getXmlFilePath()


	/**
	* Get asset name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName()
	{
		return $this->attr('name');

	}//end _getName()


//--        BRIDGE FUNCTIONS        --//


	/**
	* Returns a reference to the asset represented by the passed assetid or null on error
	*
	* @param int		$shadowid		the shadow part asset id to be loaded (i.e part after the ':')
	* @param string		$type_code		if this exists then this object is used to
	*									load the asset, if not then the DB is queried
	*									to find out the asset type
	* @param boolean	$mute_errors	stops the outputting of errors in this fn, needed because
	*									you can't use the '@' operator when returning by reference
	*
	* @return object
	* @access public
	* @see Asset_Manager::&getAsset()
	*/
	function &getAsset($shadowid, $type_code='', $mute_errors=false)
	{
		$asset = null;

		$id_parts = explode(':', $shadowid);
		if (!empty($id_parts[1])) {
			$id = $id_parts[1];

			$term = $this->getTermById($id);
			if (!empty($term)) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('thesaurus_term');
				$asset =& new Thesaurus_Term($shadowid);
			}
		}

		return $asset;

	}//end getAsset()


	/**
	* Used by asset map to return a asset map-style link array
	*
	* @return array
	* @access public
	*/
	function getAssetMapLinks()
	{
		return $this->_getShadowAssetMapLinks();

	}//end getAssetMapLinks()


	/**
	* Used by asset map to return a asset map-style link array
	*
	* @param int	$parent_term_id	The ID of the parent term
	*
	* @return array
	* @access public
	*/
	function _getShadowAssetMapLinks($parent_term_id=null)
	{

		$bridge_id = $this->id;
		$majorid = $bridge_id;
		$term_name = null;

		if (!is_null($parent_term_id)) {
			$majorid .= ':'.$parent_term_id;
			$term = $this->getTermById($parent_term_id);
			$term_name = $term['term'];
		}

		$linked_terms = $this->getChildTerms($parent_term_id);
		$links = Array();

		foreach ($linked_terms as $one_term_link) {
			$link = Array();

			// mould it all to the asset map's liking
			$link['url'] = '';
			$link['path'] = '';
			$link['num_kids'] = $this->countChildTerms($one_term_link['termid']);
			$link['accessible'] = 1;

			$link['majorid'] = $majorid;
			$link['minorid'] = $bridge_id.':'.$one_term_link['termid'];
			$link['assetid'] = $bridge_id.':'.$one_term_link['termid'];
			$link['minor_type_code'] = 'thesaurus_term';
			$link['type_code'] = 'thesaurus_term';
			$link['linkid'] = $bridge_id.':'.$one_term_link['linkid'];

			// make name and short name the same
			$name_prefix = (empty($one_term_link['relation'])?'':'('.$one_term_link['relation'].') ');
			$link['name'] = $name_prefix.$one_term_link['term'];
			$link['short_name'] = $link['name'];

			$link['status'] = $this->status;
			$link['link_type'] = SQ_LINK_TYPE_2;
			$link['sort_order'] = $one_term_link['sort_order'];

			$links[] = $link;
		}

		return $links;

	}//end _getShadowAssetMapLinks()


	/**
	* Returns an array of information about assets of the passed codes
	*
	* If one value in $field then the return value is Array(assetid => '[field]'), otherwise Array(assetid => Array())
	*
	* @param array		$assetids			an array of assets to limit
	* @param mixed		$type_code			an asset type code, or array of typecodes, to limit results to
	* @param boolean	$strict_type_code	ensure returned assets are of the passed type codes
	* @param string		$field				single field from the asset table to return (along with the assetid)
	*										Default is all fields
	*
	* @return array
	* @access public
	*/
	function getAssetInfo($assetids, $type_code=Array(), $strict_type_code=true, $field='')
	{
		$info_array = Array();

		foreach ($assetids as $id) {
			$id_parts = explode(':', $id);
			if (empty($id_parts[1])) continue;

			$term = $this->getTermById($id_parts[1]);
			if (empty($term)) continue;

			$info_array[$id]['assetid']		= $id;
			$info_array[$id]['name']		= $term['term'];
			$info_array[$id]['short_name']	= $term['term'];
			$info_array[$id]['version']		= $this->version;
			$info_array[$id]['status']		= $this->status;
			$info_array[$id]['type_code']	= 'thesaurus_term';
			$info_array[$id]['num_kids']	= $this->countChildTerms($term['termid']);
			$info_array[$id]['accessible']	= 1;
			$info_array[$id]['url']			= '';
			$info_array[$id]['web_path']	= '';

			if (!empty($field)) {
				$info_array[$id] = array_get_index($info_array[$id], $field);
			}

		}

		return $info_array;

	}//end getAssetInfo()


	/**
	* Get asset info for use by asset map
	*
	* @param string	$assetid	Full Asset id to get information for
	*
	* @return array
	* @access public
	*/
	function getAssetMapAssetInfo($assetid)
	{
		$asset_info = Array();

		$id_parts = explode(':', $assetid);
		if (empty($id_parts[1])) return $asset_info;

		$term = $this->getTermById($id_parts[1]);
		if (empty($term)) return $asset_info;

		$asset_info['assetid']		= $assetid;
		$asset_info['name']			= $term['term'];
		$asset_info['short_name']	= $term['term'];
		$asset_info['version']		= $this->version;
		$asset_info['status']		= $this->status;
		$asset_info['type_code']	= 'thesaurus_term';
		$asset_info['num_kids']		= $this->countChildTerms($term['termid']);
		$asset_info['accessible']	= 1;
		$asset_info['url']			= '';
		$asset_info['web_path']		= '';

		return $asset_info;

	}//end getAssetMapAssetInfo()


	/**
	* Return all links that this asset has to other assets
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	*/
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=true, $side_of_link='major', $sort_by=null)
	{
		return Array();

	}//end getLinks()


	/**
	* Get details of a link with the specified characteristics
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param int		$link_type			integer that should be a single integer of the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding an asset that
	*										is just a $type_code or potentially an inherited type
	*										or $type_code and any of it's sub-classes
	* @param string		$value				the value that is associated with this link
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param boolean	$exclusive			The exclusive status for the link must be
	*										this (if not null)
	*
	* @return array
	* @access public
	*/
	function getLink($assetid, $link_type=null, $type_code='', $strict_type_code=true, $value=null, $side_of_link='major', $exclusive=null)
	{
		log_dump();
		return Array();

	}//end getLink()


	/**
	* Get details of the link(s) between the two specified assets
	*
	* @param int		$assetid		id of the the $side_of_link asset
	* @param int		$other_assetid	the asset on the other side of the link
	* @param int		$link_types		integer that can be the product of bitwise operations
	*									on the SQ_LINK_* constants
	* @param string		$value			the value that is associated with this link
	* @param string		$side_of_link	Which side of the link the first assetid is on
	*									('major' or 'minor')
	* @param boolean	$force_array	force the results to return an array of links
	*									even if there is only one link
	* @param boolean	$dependant		The dependant status for all the links must be
	*									this (if not null)
	* @param boolean	$exclusive		The exclusive status for all the links must be
	*									this (if not null)
	*
	* @return array
	* @access public
	*/
	function getLinkByAsset($assetid, $other_assetid, $link_types=null, $value=null, $side_of_link='major', $force_array=false, $dependant=null, $exclusive=null)
	{
		$major_parts = explode(':',$assetid);
		$majorid = array_get_index($major_parts, 1, null);

		$minor_parts = explode(':',$other_assetid);
		$minorid = array_get_index($minor_parts, 1, null);
		if (empty($minorid)) return Array();

		$link_info = $this->getTermLinkByTermids($minorid, $majorid, $value);

		if (empty($link_info)) return Array();

		// construct template
		$link = Array(
					'majorid'			=> $assetid,
					'minorid'			=> $other_assetid,
					'link_type'			=> SQ_LINK_TYPE_2,
					'major_type_code'	=> '',
					'minor_type_code'	=> 'thesaurus_term',
					'value'				=> $value,
					'linkid'			=> $this->id.':'.$link_info['linkid'],
					'sort_order'		=> $link_info['sort_order'],
					'is_dependant'		=> 0,
					'is_exclusive'		=> 0,
				);

		return $link;

	}//end getLinkByAsset()


	/**
	* Get details of the link with specified linkid
	*
	* @param int	$linkid			id of the desired link
	* @param int	$assetid		id of the the $side_of_link asset
	*								if zero no check is made and both major and minor information is returned
	* @param string	$side_of_link	Which side of the link the asset represented by $assetid is on ('major' or 'minor')
	*
	* @return array
	* @access public
	*/
	function getLinkById($linkid, $assetid=0, $side_of_link='major')
	{
		$link_parts = explode(':', $linkid);
		$term_linkid = array_get_index($link_parts, 1);
		if (is_null($term_linkid)) return null;

		$link_info = $this->getTermLinkById($term_linkid);

		// construct template
		$link = Array(
					'majorid'			=> $this->id,
					'minorid'			=> $this->id.':'.$link_info['minor'],
					'link_type'			=> SQ_LINK_TYPE_2,
					'major_type_code'	=> 'thesaurus',
					'minor_type_code'	=> 'thesaurus_term',
					'value'				=> $link_info['relation'],
					'linkid'			=> $linkid,
					'sort_order'		=> $link_info['sort_order'],
					'is_dependant'		=> 0,
					'is_exclusive'		=> 0,
				);

		if (!is_null($link_info['major'])) {
			$link['major_type_code'] = 'thesaurus_term';
			$link['majorid'] .=':'.$link_info['major'];
		}

		return $link;

	}//end getLinkById()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = true|false
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(
									'thesaurus_term'	=> Array(
															'card'		=> 'M',
															'exclusive'	=> false,
														   ),
								   ),
				SQ_LINK_TYPE_3	=> Array(),
			   );

	}//end _getAllowedLinks()


	/**
	* Get all asset ids that are below the passed assetid in the various trees in which it exists
	*
	* Returns an array of assetids and their type code
	*
	* @param int		$assetid			the id of the asset to get its children for
	* @param mixed		$type_code			the type of asset that is linked
	*										(eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='', $strict_type_code=true, $sort_by=null)
	{
		return Array();
	}//end getChildren()


	/**
	* Create a link between the two supplied assets
	*
	* returns the linkid of the created link
	*
	* @param object		&$major		the major asset that we are linking from
	* @param object		&$minor		the minor asset that we are linking to
	* @param string		$link_type	the type of link this is
	* @param string		$value		the value that is to be associated with this link
	* @param string		$sort_order	the position in the links list that this link should take,
	*								if null or less than zero places at end of list
	* @param string		$dependant	'0' / '1' - whether the minor asset is dependant on the major
	* @param string		$exclusive	'0' / '1' - whether the major asset is to be the minor's only parent
	* @param boolean	$moving		whether or not this asset is being moved
	*
	* @return string
	* @access public
	*/
	function createAssetLink(&$major, &$minor, $link_type, $value='', $sort_order=null, $dependant='0', $exclusive='0', $moving=false)
	{
		$major_parts = explode(':',$major->id);
		$majorid = array_get_index($major_parts, 1);

		$minor_parts = explode(':',$minor->id);
		$minorid = array_get_index($minor_parts, 1);
		if (empty($minorid)) return null;

		$relation = empty($value)?null:$value;
		$minor_term = null;
		$major_term = null;

		// can only link terms which belong to this thesaurus
		if (($minor_parts[0] != $this->id) || ($major_parts[0] != $this->id)) {
			return 0;
		}

		$minor_term = $minor->name;

		if (isset($major_parts[1])) {
			$major_term = $major->name;
		}

		if ($moving) {
			$link_info = $this->getTermLinkByTermids($minorid, $majorid, $relation);
			if (!is_null($link_info)) {
				trigger_localised_error('CORE0240');
				return false;
			}
		}

		$term_linkid = $this->addTerm($minor_term, $major_term, $relation, $sort_order);

		if (is_null($term_linkid)) return false;

		return $this->id.':'.$term_linkid;

	}//end createAssetLink()


	/**
	* Returns an array of all user or user_group assets that have the passed permission for the passed asset
	*
	* @param int		$assetid		the assetid for asset whose permissions to get
	* @param string		$permission		the permission code you are getting
	* @param boolean	$granted		type of Access : null = all, true = granted, false = denied
	* @param boolean	$and_greater	get effective permission (eg read access = read
	*									or write or admin)
	* @param boolean	$expand_groups	expand user groups so only user ids get returned
	*									(NOTE: only valid if $all_info is false)
	* @param boolean	$all_info		when false fn just returns an array of userids for
	*									those that have permission
	*									When true fn returns all info about the permission in the form of
	*									Array(userid => granted)
	*									(NOTE: TRUE is only valid if $and_greater and
	*									$expand_groups are false)
	*
	* @return array
	* @access public
	*/
	function getPermission($assetid, $permission, $granted=null, $and_greater=true, $expand_groups=false, $all_info=false)
	{
		return Array();
	}//end getPermission()


	/**
	* Mark thesaurus as having its contents changed
	*
	* This function should be called when any change to thesaurus is performed
	* XML output functionality depends on this
	*
	* @return void
	* @access public
	*/
	function markContentsChanged()
	{
		if ($this->attr('contents_changed')) return;

		$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
		$this->setAttrValue('contents_changed',true);
		$this->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		return;

	}//end markContentsChanged()


	/**
	* Return the number of links in the system that involve the specified asset
	*
	* Can be restricted by either link type afnd/or asset type code
	*
	* @param int		$assetid			id of the the $side_of_link asset
	* @param string		$side_of_link		Which side of the link this (the current) asset
	*										is on ('major' or 'minor')
	* @param int		$link_types			integer that can be the product of bitwise operations
	*										on the SQ_LINK_* constants
	* @param mixed		$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*										if an array returns link if matches any
	*										of the array values
	* @param boolean	$strict_type_code	whether we are finding assets that are just a $type_code
	*										or $type_code and any of it's sub-classes
	* @param int		$ignore_linkid		ignore the link represented by this link
	*										id when returning the count
	*
	* @return int
	* @access public
	*/
	function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=true, $ignore_linkid=0)
	{

		$id_parts = explode(':', $assetid);

		$termid = array_get_index($id_parts, 1);
		if (is_null($termid)) return -1;

		$relations = Array();

		if ($side_of_link == 'major') {
			$keyword = 'Child';
		} else {
			$keyword = 'Parent';
		}
		$get_term_function = 'count'.$keyword.'Terms';
		return $this->$get_term_function($termid);

	}//end countLinks()


	/**
	* Remove a link by id
	*
	* @param int		$linkid	the link id of the link to remove
	* @param boolean	$moving	TRUE if this delete is part of a move operation
	*
	* @return boolean
	* @access public
	*/
	function deleteAssetLink($linkid, $moving=false)
	{
		$link_parts = explode(':', $linkid);
		if (isset($link_parts[1])) {
			$this->deleteTermLink($link_parts[1]);
		}
		return true;

	}//end deleteAssetLink()


	/**
	* Delete a link between two terms
	*
	* @param int	$linkid	link id to remove
	*
	* @return void
	* @access public
	*/
	function deleteTermLink($linkid)
	{
		if (empty($linkid)) return false;

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// get the list of child linkids that MAY be deleted
		$sql = '
			SELECT
				DISTINCT(Btree.linkid)
			FROM
				sq_thes_lnk_tree Atree,
				sq_thes_lnk_tree Btree
			WHERE
				Atree.linkid = '.$db->quoteSmart($linkid).'
				AND
				Btree.treeid LIKE (Atree.treeid || '.$db->quoteSmart('%').')
				AND
				Btree.treeid > Atree.treeid
				AND
				Atree.thesid = '.$db->quoteSmart($this->id).'
				AND
				Btree.thesid = '.$db->quoteSmart($this->id).'
		';

		$candidates_to_delete = $db->getCol($sql);
		assert_valid_db_result($candidates_to_delete);

		$links_to_delete = Array();

		if (!empty($candidates_to_delete)) {

			$clean_linkids = Array();
			foreach ($candidates_to_delete as $treeid) {
				$clean_linkids[] = $db->quoteSmart($treeid);
			}

			// select link ids that WILL NOT be deleted
			$sql = '
				SELECT
					DISTINCT(Btree.linkid)
				FROM
					sq_thes_lnk_tree Atree,
					sq_thes_lnk_tree Btree
				WHERE
					Atree.linkid = '.$db->quoteSmart($linkid).'
					AND
					Btree.treeid NOT LIKE (Atree.treeid || '.$db->quoteSmart('%').')
					AND
					Btree.linkid IN ('.implode(',', $clean_linkids).')
					AND
					Atree.thesid = '.$db->quoteSmart($this->id).'
					AND
					Btree.thesid = '.$db->quoteSmart($this->id).'
			';
			$links_to_keep = $db->getCol($sql);
			assert_valid_db_result($links_to_keep);

			$raw_links_to_delete = array_diff($candidates_to_delete, $links_to_keep);
			foreach ($raw_links_to_delete as $id) {
				$links_to_delete[$id] = $db->quoteSmart($id);
			}
		}
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$sql = '
			DELETE FROM
				sq_thes_lnk_tree
			WHERE
				treeid IN (
					SELECT
						Btree.treeid
					FROM
						sq_thes_lnk_tree Atree,
						sq_thes_lnk_tree Btree
					WHERE
						Atree.linkid = '.$db->quoteSmart($linkid).'
						AND
						Btree.treeid LIKE (Atree.treeid || '.$db->quoteSmart('%').')
						AND
						Btree.treeid > Atree.treeid
						AND
						Atree.thesid = '.$db->quoteSmart($this->id).'
						AND
						Btree.thesid = '.$db->quoteSmart($this->id).'
				)
				AND
				thesid = '.$db->quoteSmart($this->id).'
		';

		$result = $db->query($sql);
		assert_valid_db_result($result);

		// mark the deleted treeids as having linkid '0' for recycling
		$sql = '
			UPDATE
				sq_thes_lnk_tree
			SET
				linkid = '.$db->quoteSmart('0').'
			WHERE
				linkid = '.$db->quoteSmart($linkid).'
				AND
				thesid = '.$db->quoteSmart($this->id).'
		';

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$links_to_delete[$linkid] = $db->quoteSmart($linkid);

		// update the sort order
		foreach ($links_to_delete as $id => $quoted_linkid) {
			$link_to_delete = $this->getTermLinkById($id);
			$sql = '
				UPDATE
					sq_thes_lnk
				SET
					sort_order = sort_order - 1
				WHERE
					major = '.$db->quoteSmart($link_to_delete['major']).'
					AND
					sort_order > '.$db->quoteSmart($link_to_delete['sort_order']).'
					AND
					thesid = '.$db->quoteSmart($this->id).'
			';
			$result = $db->query($sql);
			assert_valid_db_result($result);
		}

		// delete terms that will be stranded after the deletion of this link
		// NOTE: This query will need to be taken apart when we need to broadcast events BEFORE and AFTER deletion
		$sql = '
			DELETE FROM
				sq_thes_term
			WHERE
				termid IN
				(
					SELECT
						finished.minor
					FROM
						sq_thes_lnk finished
						INNER JOIN sq_thes_lnk selected ON finished.minor = selected.minor
					WHERE
						selected.linkid IN ('.implode(',', $links_to_delete).')
						AND
						selected.minor NOT IN
						(
							SELECT
								surviving.minor
							FROM
								sq_thes_lnk surviving
								INNER JOIN sq_thes_lnk selected ON surviving.minor = selected.minor,
								sq_thes_lnk every
							WHERE
								selected.linkid IN ('.implode(',', $links_to_delete).')
								AND
								surviving.minor = every.minor
								AND
								every.linkid <> selected.linkid

								AND
								surviving.thesid = '.$db->quoteSmart($this->id).'
								AND
								selected.thesid = '.$db->quoteSmart($this->id).'
								AND
								every.thesid = '.$db->quoteSmart($this->id).'
						)
						AND
						selected.thesid = '.$db->quoteSmart($this->id).'
						AND
						finished.thesid = '.$db->quoteSmart($this->id).'
				)
				AND
				thesid = '.$db->quoteSmart($this->id).'
		';
		$result = $db->query($sql);
		assert_valid_db_result($result);

		// finally delete all links
		$sql = '
			DELETE FROM
				sq_thes_lnk
			WHERE
				linkid in
				('.implode(',', $links_to_delete).')
				AND
				thesid = '.$db->quoteSmart($this->id).'
		';

		$result = $db->query($sql);
		assert_valid_db_result($result);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end deleteTermLink()


	/**
	* Move a link to a new parent
	*
	* @param int	$linkid			the link id of the link to move
	* @param int	$to_parentid	the asset id of the new parent asset
	* @param int	$link_type		the type of the new link
	* @param int	$to_parent_pos	the position under the new parent
	*
	* @return int
	* @access public
	*/
	function moveLink($linkid, $to_parentid, $link_type, $to_parent_pos)
	{
		log_dump();
	}//end moveLink()


	/**
	* Update the details of an existing link
	*
	* If any of the detail vars are NULL they are not updated.
	*
	* @param int	$linkid			the link id of the link to update
	* @param int	$link_type		the type of the link (one of the SQ_LINK_* constants)
	* @param string	$value			the value to place on the link
	* @param int	$new_sort_order	the position in the links list that this link should take,
	*								if less than zero places at end of list
	*
	* @return boolean
	* @access public
	*/
	function updateLink($linkid, $link_type=null, $value=null, $new_sort_order=null)
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$linkid_parts = explode(':', $linkid);
		if (!isset($linkid_parts[1]) || ($linkid_parts[0] != $this->id)) {
			return false;
		}

		$term_linkid = $linkid_parts[1];
		$link_info = $this->getTermLinkById($term_linkid);
		if (empty($term)) return false;

		$current_sort_order = $link_info['sort_order'];
		$parentid = $link_info['major'];

		if ($current_sort_order == $new_sort_order) {
			return true;
		}

		$max_sort_order = $this->getMaxSortOrder($parentid);

		if (($new_sort_order > $max_sort_order) || ($new_sort_order < 0)) {
			$new_sort_order = $max_sort_order;
		}

		if ($new_sort_order > $current_sort_order) {
			$shift_sql = '
				UPDATE
					sq_thes_lnk
				SET
					sort_order = sort_order - 1
				WHERE
					sort_order > '.$db->quoteSmart($current_sort_order).'
					AND
					sort_order <= '.$db->quoteSmart($new_sort_order).'
					AND
					major '.(is_null($parentid)?'ISNULL':'= '.$db->quoteSmart($parentid)).'
					AND
					thesid = '.$db->quoteSmart($this->id).'
			';
		} else {
			$shift_sql = '
				UPDATE
					sq_thes_lnk
				SET
					sort_order = sort_order + 1
				WHERE
					sort_order < '.$db->quoteSmart($current_sort_order).'
					AND
					sort_order >= '.$db->quoteSmart($new_sort_order).'
					AND
					major '.(is_null($parentid)?'ISNULL':'= '.$db->quoteSmart($parentid)).'
					AND
					thesid = '.$db->quoteSmart($this->id).'
			';
		}

		$update_sql = '
			UPDATE
				sq_thes_lnk
			SET
				sort_order = '.$db->quoteSmart($new_sort_order).'
			WHERE
				linkid = '.$db->quoteSmart($term_linkid).'
				AND
				thesid = '.$db->quoteSmart($this->id).'
		';

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			$result = $db->query($shift_sql);
			assert_valid_db_result($result);

			$result = $db->query($update_sql);
			assert_valid_db_result($result);
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

		return true;

	}//end updateLink()


	/**
	* Get the maximum sort order value under a major term
	*
	* @param string	$majorid	Major term id
	*
	* @return mixed int|null
	* @access public
	*/
	function getMaxSortOrder($majorid=null)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				max(sort_order)
			FROM
				sq_thes_lnk
			WHERE
				major '.(is_null($majorid)?'ISNULL':'= '.$db->quoteSmart($majorid)).'
				AND
				thesid = '.$db->quoteSmart($this->id).'
		';

		$max_sort_order = $db->getOne($sql);
		assert_valid_db_result($max_sort_order);

		return $max_sort_order;

	}//end getMaxSortOrder()


	/**
	* Get all lineages for a term
	*
	* @param string	$term	the name of the term
	*
	* @return array
	* @access public
	*/
	function getLineagesForTerm($term)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		// Get an array of arrays of lineages.
		$sql = '
			SELECT
				child_tree.treeid 		as our_treeid,
				parent_term.relation	as relation,
				parent_term.minor 		as term,
				parent_tree.treeid 		as parent_treeid
			FROM
				sq_thes_term child_term
				INNER JOIN sq_thes_lnk_tree child_tree ON child_term.linkid = child_tree.linkid,
				sq_thes_term parent_term
				INNER JOIN sq_thes_lnk_tree parent_tree ON parent_term.linkid = parent_tree.linkid
			WHERE
				child_tree.treeid IN (
					SELECT
						result_tree.treeid as child_treeid
					FROM
						sq_thes_lnk_tree test_tree
						INNER JOIN sq_thes_term test_term ON test_tree.linkid = test_term.linkid,
						sq_thes_lnk_tree result_tree
						INNER JOIN sq_thes_term result_term ON result_tree.linkid = result_term.linkid
					WHERE
						result_tree.treeid LIKE (test_tree.treeid || '.$db->quoteSmart('%').')
						AND
						test_term.minor = '.$db->quoteSmart($term).'
						AND
						result_term.minor = '.$db->quoteSmart($term).'
						AND
						test_tree.thesid = '.$db->quoteSmart($this->id).'
						AND
						result_tree.thesid = '.$db->quoteSmart($this->id).'
						AND
						test_term.thesid = '.$db->quoteSmart($this->id).'
						AND
						result_term.thesid = '.$db->quoteSmart($this->id).'
					GROUP BY
						result_tree.treeid
					HAVING
						count(result_tree.treeid) =1
				)
				AND
				child_tree.treeid LIKE parent_tree.treeid || '.$db->quoteSmart('%').'
				AND
				parent_tree.treeid <= child_tree.treeid
				AND
				parent_tree.thesid = '.$db->quoteSmart($this->id).'
				AND
				child_tree.thesid = '.$db->quoteSmart($this->id).'
			ORDER BY
				child_term.linkid,
				child_tree.treeid,
				parent_tree.treeid
		';

		$result = $db->getAssoc($sql, false, Array(), DB_FETCHMODE_ASSOC, true);
		assert_valid_db_result($result);

		return $result;
	}//end getLineagesForTerm()


}//end class

?>