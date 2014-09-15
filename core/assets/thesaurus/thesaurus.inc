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
* $Id: thesaurus.inc,v 1.69 2012/08/30 01:09:16 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_CORE_PACKAGE_PATH.'/interfaces/bridge/bridge.inc';

/**
* Thesaurus
*
* NOTE: All term comparisons are case insensitive. Term case is preserved.
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Elden McDonald <emcdonald@squiz.net>
* @version $Revision: 1.69 $

* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Thesaurus extends Asset implements Bridge
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Thesaurus($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Delete Asset
	*
	* @param boolean	$release_lock	TRUE, to release lock
	*
	* @return boolean
	* @access public
	*/
	function delete($release_lock)
	{
		if (parent::delete($release_lock)) {
			$this->_deleteStoredContents();
			return TRUE;
		}

		return FALSE;

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
		if (!parent::_createAdditional($link)) return FALSE;
		return create_directory($this->data_path);

	}//end _createAdditional()


//--		BUILT-IN RELATION SUPPORT		--//


	/**
	* Get an array of abbrevations
	*
	* the result is an array of format ("A" => "Australia")
	* abbreviation => expansion
	* NOTE: An expansion can have only one abbreviation
	*
	* @return array
	* @access public
	*/
	function getAbbreviations()
	{
		$abbreviation_attr = $this->getAttribute('abbreviation_rel');
		$abbreviation_rel = $abbreviation_attr->value;
		if (empty($abbreviation_rel)) {
			$abbreviation_rel = NULL;
		}

		$abbr_rows = $this->getRelatedTerms($abbreviation_rel);

		if (is_null($abbr_rows)) return Array();

		$result = Array();
		foreach ($abbr_rows as $abbr) {
			$result[$abbr['major']] = $abbr['minor'];
		}

		return $result;

	}//end getAbbreviations()


	/**
	* Get synonyms for a given term
	*
	* @param string	$term	Term to find synonyms for
	*
	* @return void
	* @access public
	*/
	function getSynonymsForTerm($term)
	{
		$termid = $this->getTermIdByName($term);

		if (empty($termid)) return Array();

		$synonym_attr = $this->getAttribute('synonym_rel');
		$synonym_rel = $synonym_attr->value;
		if (empty($synonym_rel)) $synonym_rel = NULL;

		$synonyms = $this->getChildTerms($termid, $synonym_rel);
		$result = Array();

		if (!empty($synonyms)) {
			foreach ($synonyms as $synonym) {
				$result[$synonym['term']] = $synonym['term'];
			}
		}

		return $result;

	}//end getSynonymsForTerm()


//--        CORE THESAURUS RELATIONAL ENGINE        --//


	/**
	* Get minor terms
	*
	* Returns an array of terms which have a given term as their parent
	*
	* @param int	$termid	Term
	* @param string	$relid	Relation name to limit the lookup by
	*
	* @return array
	* @access public
	*/
	function getChildTerms($termid, $relid=NULL)
	{

		$limit_relation = TRUE;
		if (is_null($relid)) $limit_relation = FALSE;

		$db = MatrixDAL::getDb();

		$sql = '
			SELECT
				linkid, term, termid, relid
			FROM
				sq_thes_term term
				INNER JOIN sq_thes_lnk link ON term.termid = link.minor
			WHERE
				term.thesid = :assetid_1
				AND
				link.thesid = :assetid_2
				AND
				major = :termid
		';

		if ($limit_relation) {
			$sql .= '
				AND
				relid = :relid
			';
		}

		$sql .= '
			ORDER BY
				term
		';

		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'assetid_1', $this->id);
		MatrixDAL::bindValueToPdo($query, 'assetid_2', $this->id);
		MatrixDAL::bindValueToPdo($query, 'termid', $termid);
		if ($limit_relation) {
			MatrixDAL::bindValueToPdo($query, 'relid', $relid);
		}//end if

		try {
			$result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get thesaurus child terms due to database error: '. $e->getMessage());
		}//end

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

		$db = MatrixDAL::getDb();

		$bind_vars = Array (
						'assetid'	=> $this->id,
						'termid'	=> $termid,
					);

		try {
			$result = MatrixDAL::executeOne('thesaurus', 'countChildTerms', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to count thesaurus child terms due to database error: '. $e->getMessage());
		}//end

		return $result;

	}//end countChildTerms()


	/**
	* Count the links in a thesaurus
	*
	* @return array
	* @access public
	*/
	function countTermLinks()
	{

		$db = MatrixDAL::getDb();

		$bind_vars['assetid'] = $this->id;

		try {
			$result = MatrixDAL::executeOne('thesaurus', 'countTermLinks', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to count thesaurus term links due to database error: '. $e->getMessage());
		}//end

		return $result;

	}//end countTermLinks()


	/**
	* Count terms a thesaurus
	*
	* @return array
	* @access public
	*/
	function countTerms()
	{

		$db = MatrixDAL::getDb();

		$bind_vars['assetid'] = $this->id;

		try {
			$result = MatrixDAL::executeOne('thesaurus', 'countTerms', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to count thesaurus terms due to database error: '. $e->getMessage());
		}//end


		return $result;

	}//end countTerms()


	/**
	* Count parent terms
	*
	* Get a count of how many terms are linked directly above a given term
	*
	* @param int	$termid	Term
	*
	* @return array
	* @access public
	*/
	function countParentTerms($termid)
	{

		$db = MatrixDAL::getDb();

		$bind_vars = Array (
						'assetid'   => $this->id,
						'termid'    => $termid,
					);

		try {
			$result = MatrixDAL::executeOne('thesaurus', 'countParentTerms', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to count thesaurus parent terms due to database error: '. $e->getMessage());
		}//end


		return $result;

	}//end countParentTerms()


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
	function getChildRelationsForTerm($termid)
	{

		$db = MatrixDAL::getDb();

		$bind_vars = Array (
						'assetid'   => $this->id,
						'termid'    => $termid,
					);

		try {
			$results = MatrixDAL::executeAssoc('thesaurus', 'getChildRelationsForTerm', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to get child relations for term due to database error: '. $e->getMessage());
		}//end
		if (!empty($results)) {
			$new_res = Array();
			foreach ($results as $result) {
				$new_res[$result['relid']] = $result['relation'];
			}//end foreach
			$results = $new_res;
		}//end if

		return $results;

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
	function getParentRelationsForTerm($termid)
	{

		if (!isset($termid)) return NULL;

		$db = MatrixDAL::getDb();

		$bind_vars = Array (
						'assetid'   => $this->id,
						'termid'    => $termid,
					);

		try {
			$result = MatrixDAL::executeAssoc('thesaurus', 'getParentRelationsForTerm', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to get parent relations for term due to database error: '. $e->getMessage());
		}//end

		return $result;

	}//end getParentRelationsForTerm()


	/**
	* Get a list of all terms
	*
	* @return array
	* @access public
	*/
	function getAllTerms()
	{
		$bind_vars	= Array();
		$bind_vars['assetid'] = $this->id;

		try {
			$result = MatrixDAL::executeAll('thesaurus', 'getAllTerms', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to get a list of all thesaurus terms due to database error: '. $e->getMessage());
		}//end

		return $result;

	}//end getAllTerms()


	/**
	* Get parent terms
	*
	* Return terms which are immediate parents of a given term
	*
	* @param int	$termid	Term ID
	* @param int	$relid	Relation ID
	*
	* @return array
	* @access public
	*/
	function getParentTerms($termid, $relid=NULL)
	{

		$limit_relation = TRUE;
		if (is_null($relid)) $limit_relation = FALSE;

		$db = MatrixDAL::getDb();

		$sql = '
			SELECT
				linkid, term, termid, relid
			FROM
				sq_thes_term term
				INNER JOIN sq_thes_lnk link ON term.termid = link.major
			WHERE
				term.thesid = :thesid_1
				AND
				link.thesid = :thesid_2
				AND
				minor = :termid
		';

		if ($limit_relation) {
			$sql .= '
				AND
				relid = :relid
			';
		}

		$sql .= '
			ORDER BY
				term
		';


		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'thesid_1', $this->id);
		MatrixDAL::bindValueToPdo($query, 'thesid_2', $this->id);
		MatrixDAL::bindValueToPdo($query, 'termid', $termid);
		if ($limit_relation) {
			MatrixDAL::bindValueToPdo($query, 'relid', $relid);
		}//end if

		try {
			$result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get thesaurus parent terms due to database error: '. $e->getMessage());
		}//end

		return $result;

	}//end getParentTerms()


	/**
	* Given a relation, get all the Parent-Child pairs
	*
	* @param string	$relation_id	Relation to lookup
	*
	* @return void
	* @access public
	*/
	function getRelatedTerms($relation_id='')
	{
		if (empty($relation_id)) return NULL;

		$db = MatrixDAL::getDb();

		$sql = '
			SELECT
				major.term		as major,
				major.termid	as majorid,
				minor.term		as minor,
				minor.termid	as minorid
			FROM
				sq_thes_term major
				INNER JOIN sq_thes_lnk link ON major.termid = link.major
				INNER JOIN sq_thes_term minor ON minor.termid = link.minor
			WHERE
				link.relid = :relid
				AND
				major.thesid = :major_thesid
				AND
				minor.thesid = :minor_thesid
				AND
				link.thesid = :link_thesid
		';

		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'relid', $relation_id);
		MatrixDAL::bindValueToPdo($query, 'major_thesid', $this->id);
		MatrixDAL::bindValueToPdo($query, 'minor_thesid', $this->id);
		MatrixDAL::bindValueToPdo($query, 'link_thesid',  $this->id);
		$result = MatrixDAL::executePdoAll($query);

		return $result;

	}//end getRelatedTerms()


	/**
	* Helper function to add a term to the system
	*
	* NOTE: Does NOT check if the term already exists
	*
	* @param string	$term	Term
	*
	* @return mixed NULL|int
	* @access private
	*/
	function _addTerm($term)
	{
		if (!isset($term) || $term == '') {
			return NULL;
		}

		// remove leading and trailing spaces
		$term = trim($term);

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

		$termid = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_thes_term_seq'));
		$bind_vars = Array (
						'thesid'	=> $this->id,
						'term'		=> $term,
						'termid'	=> $termid,
					);

		try {
			$result = MatrixDAL::executeQuery('thesaurus', 'insertTerm', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to insert a thesaurus term due to database error: '. $e->getMessage());
		}//end

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $termid;

	}//end _addTerm()


	/**
	* Add a term to the system
	*
	* Wrapper around '_addTerm' that checks if the term already exists
	*
	* @param string	$term	Term
	*
	* @return mixed NULL|int
	* @access private
	*/
	function addTerm($term)
	{
		$termid = $this->getTermIdByName($term);
		if (!is_null($termid)) return $termid;

		return $this->_addTerm($term);

	}//end addTerm()


	/**
	* Add a term note
	*
	* @param string	$termid		Term ID
	* @param string	$note_name	Note Name
	* @param string	$note_value	Note Value
	*
	* @return array
	* @access public
	*/
	function addTermNote($termid, $note_name, $note_value)
	{
		// before adding the Term Note we need to check if the note hasnt been added
		// if the term note is already in the table we will throw an error
		$db = MatrixDAL::getDb();
		$sql = 'SELECT a.termid, a.name, a.value, a.thesid, b.term
				FROM sq_thes_term_note a, sq_thes_term b
				WHERE a.termid = '.MatrixDAL::quote($termid).' AND
					a.name = '.MatrixDAL::quote($note_name).' AND
					a.value = '.MatrixDAL::quote($note_value).' AND
					a.thesid = '.MatrixDAL::quote($this->id).' AND
					b.thesid = a.thesid AND b.termid = a.termid
					';
		$result = NULL;
			try {
			$query = MatrixDAL::preparePdoQuery($sql);
			$result = MatrixDAL::executePdoAssoc($query);
		} catch (Exception $e) {
			throw new Exception($e->getMessage());
		}

		if (!empty($result)) {
			// throw an error and return TRUE
			trigger_localised_error('CORE0282', E_USER_WARNING, $note_name.":".$note_value, $result[0]['term']);
			return TRUE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');


		$bind_vars = Array (
						'thesid'	=> $this->id,
						'termid'	=> $termid,
						'name'		=> $note_name,
						'value'		=> $note_value,
					);

		try {
			$result = MatrixDAL::executeQuery('thesaurus', 'insertTermNote', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to insert a thesaurus term note due to database error: '. $e->getMessage());
		}//end

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end addTermNote()


	/**
	* Get All Term Notes
	*
	* @param string	$termid	Term ID
	*
	* @return array
	* @access public
	*/
	function getTermNotes($termid)
	{

		$db = MatrixDAL::getDb();

		$bind_vars = Array (
						'assetid'   => $this->id,
						'termid'    => $termid,
					);

		try {
			$result = MatrixDAL::executeAssoc('thesaurus', 'getTermNotes', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to get thesaurus term note due to database error: '. $e->getMessage());
		}//end

		return $result;

	}//end getTermNotes()


	/**
	* Delete All notes for a particular term
	*
	* @param string	$termid	Term ID
	*
	* @return array
	* @access public
	*/
	function deleteTermNotes($termid)
	{
		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');

		$db = MatrixDAL::getDb();

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$bind_vars = Array (
						'thesid'    => $this->id,
						'termid'    => $termid,
					);

		try {
			$result = MatrixDAL::executeQuery('thesaurus', 'deleteTermNotes', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete a thesaurus term note due to database error: '. $e->getMessage());
		}//end


		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end deleteTermNotes()


	/**
	* Link two terms in the system
	*
	* Wrapper around '_linkTerms' that checks if this link already exists
	*
	* @param string	$minorid	minor term id
	* @param string	$majorid	major term id
	* @param string	$relid		relation id
	*
	* @return mixed NULL|int
	* @access private
	*/
	function linkTerms($minorid, $majorid, $relid)
	{
		$term_linkid = $this->_linkTerms($minorid, $majorid, $relid);

		if (is_null($term_linkid)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return NULL;
		}

		if ($this->attr('enforce_reflection')) {
			$ref_relid = $this->getReflectionForRelation($relid);
			if ($ref_relid) {
				$this->_linkTerms($majorid, $minorid, $ref_relid);
			}
		}

		return $term_linkid;

	}//end linkTerms()


	/**
	* Link two terms together
	*
	* NOTE: This function does not check if the terms exist
	* NOTE: Does not check if the LINK already EXISTS
	*
	* @param string	$minorid		Minor term
	* @param string	$majorid		Major term, if NULL, linking to the top
	* @param string	$relation_id	Relation
	*
	* @return mixed NULL|int
	* @access public
	*/
	function _linkTerms($minorid, $majorid, $relation_id=NULL)
	{
		if (is_null($minorid) || is_null($majorid)) {
			return NULL;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();

		$bind_vars = Array (
						'thesid'    => $this->id,
						'major'     => $majorid,
						'minor'     => $minorid,
						'relation'  => $relation_id,
					 );

		try {
			$result = MatrixDAL::executeOne('thesaurus', 'getLinkId', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to get thesaurus linkid due to database error: '. $e->getMessage());
		}//end


		// if the terms are already linked using this relation, just return the ID
		if (!empty($result)) return $result;

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// inserting a term link is the easiest thing ever
		try {
			$linkid = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_thes_lnk_seq'));
			$bind_vars['linkid']	= $linkid;
			$result = MatrixDAL::executeQuery('thesaurus', 'insertLink', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to link a thesaurus term due to database error: '. $e->getMessage());
		}//end


		$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
		$major = $this->getAsset($majorid);
		$em->broadcastEvent($major, 'CreateLink', Array());

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return $linkid;

	}//end _linkTerms()


	/**
	* Update term name and relation
	*
	* Returns TRUE on success, FALSE on failure
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

		// remove leading and trailing spaces
		$name = trim($name);

		// if nothing has changed, no need to do any work
		if ($term['term'] == $name) return TRUE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();


		// if the new name is being used by another term, do not update
		// updating the name if it is used by another term is hard as we don't know what to do with the children
		// do we merge this term's children with the others' or do we only use our children or theirs?
		if ($this->isTerm($name, $termid)) {
			trigger_localised_error('CORE0239', E_USER_WARNING);
			return FALSE;
		}


		// if the term has only one parent and the name is not used anywhere else - it is safe to update it
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		// update the term itself

		$bind_vars = Array (
						'assetid'	=> $this->id,
						'termid'	=> $termid,
						'name'		=> $name,
					 );

		try {
			$result = MatrixDAL::executeQuery('thesaurus', 'updateTerm', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to update a thesaurus term due to database error: '. $e->getMessage());
		}//end


		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		return TRUE;

	}//end updateTerm()


	/**
	* Renames all instances of $relation to $new_relation
	*
	* Returns TRUE on success, FALSE on failure
	*
	* @param string	$relation_id	relation to rename
	* @param string	$new_name		the desired new relation
	*
	* @return boolean
	* @access public
	*/
	function renameRelation($relation_id, $new_name)
	{
		if ($new_name == '') $new_name = NULL;

		// IMPORTANT: Ensure the new name is not already in use
		$relid = $this->getRelationIdByName($new_name);
		if (!is_null($relid)) return NULL;

		if (array_search($new_name, $this->getActiveRelations())!== FALSE) {
			// can't rename an existing relation
			$this->_tmp['error'][] = translate('thesaurus_new_relation_in_use');
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// Update relations

		$bind_vars = Array (
						'assetid'	=> $this->id,
						'relid'		=> $relation_id,
						'relation'	=> $new_name,
					 );

		try {
			$result = MatrixDAL::executeQuery('thesaurus', 'updateRelationName', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to update a thesaurus relation name due to database error: '. $e->getMessage());
		}//end

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

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
		$db = MatrixDAL::getDb();

		$bind_vars = Array (
						'assetid'	=> $this->id,
						'termid'	=> $termid,
					);

		try {
			$result = MatrixDAL::executeAll('thesaurus', 'getTermById', $bind_vars);
			if (!empty($result) && isset($result[0])) {
				$result = $result[0];
			}
		} catch (Exception $e) {
			throw new Exception('Unable to get a thesaurus term by id due to database error: '. $e->getMessage());
		}//end

		return $result;

	}//end getTermById()


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
		$db = MatrixDAL::getDb();

		$bind_vars = Array (
						'assetid'   => $this->id,
						'linkid'    => $linkid,
					 );

		try {
			$result = MatrixDAL::executeAll('thesaurus', 'getLinkById', $bind_vars);
			if (!empty($result) && isset($result[0])) {
				$result = $result[0];
			}
		} catch (Exception $e) {
			throw new Exception('Unable to get a thesaurus term link by id due to database error: '. $e->getMessage());
		}//end

		return $result;

	}//end getTermLinkById()


	/**
	* Get relation ID given the name
	*
	* @param string	$name	relation name
	*
	* @return void
	* @access public
	*/
	function getRelationIdByName($name)
	{
		$db = MatrixDAL::getDb();

		$sql = '
			SELECT
				relid
			FROM
				sq_thes_rel
			WHERE
				relation '.(is_null($name)?'IS NULL':'= :name').'
				AND
				thesid = :assetid';

		$query = MatrixDAL::preparePdoQuery($sql);
		if (!is_null($name)) {
			MatrixDAL::bindValueToPdo($query, 'name', $name);
		}
		MatrixDAL::bindValueToPdo($query, 'assetid', $this->id);

		try {
			$result = MatrixDAL::executePdoOne($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get thesaurus relation Id by name due to database error: '. $e->getMessage());
		}//end

		if (empty($result)) $result = NULL;

		return $result;

	}//end getRelationIdByName()


	/**
	* Get relation ID given the name
	*
	* @param string	$name	relation name
	*
	* @return string
	* @access public
	*/
	function addRelation($name)
	{
		if ($name == '') $name = NULL;

		// IMPORTANT: Ensure the new name is not already in use
		$relid = $this->getRelationIdByName($name);
		if (!is_null($relid)) return $relid;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$db = MatrixDAL::getDb();

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$id = MatrixDAL::executeOne('core', 'seqNextVal', Array('seqName' => 'sq_thes_rel_seq'));
		$bind_vars = Array (
						'thesid'	=> $this->id,
						'relation'	=> $name,
						'relid'		=> $id,
					 );

		try {
			$result = MatrixDAL::executeQuery('thesaurus', 'insertRelation', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to add a thesaurus relation due to database error: '. $e->getMessage());
		}//end

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return $id;

	}//end addRelation()


	/**
	* Update link with a new relation id
	*
	* @param string	$linkid	relation name
	* @param string	$relid	relation name
	*
	* @return void
	* @access public
	*/
	function updateLinkRelation($linkid, $relid)
	{

		if (!isset($linkid) || !isset($relid)) {
			return FALSE;
		}

		$link_info = $this->getTermLinkById($linkid);
		$major = $link_info['major'];
		$minor = $link_info['minor'];
		$old_relid = $link_info['relid'];

		$this->deleteTermLink($linkid);
		$this->linkTerms($minor, $major, $relid);

		return TRUE;

	}//end updateLinkRelation()


	/**
	* Get link by child and parent term ids
	*
	* @param string	$termid		the id of the child in this link
	* @param string	$parentid	the id of the parent in this link
	* @param string	$relid		the relation of the link
	*
	* @return void
	* @access public
	*/
	function getTermLinkByTermids($termid, $parentid, $relid=NULL)
	{

		$db = MatrixDAL::getDb();

		$bind_vars = Array (
						'thesid'	=> $this->id,
						'relid'		=> $relid,
						'parentid'	=> $parentid,
						'termid'	=> $termid,
					 );

		try {
			$result = MatrixDAL::executeAll('thesaurus', 'getTermLinkByTermIds', $bind_vars);
			if (!empty($result) && isset($result[0])) {
				$result = $result[0];
			}
		} catch (Exception $e) {
			throw new Exception('Unable to get a thesaurus term link by id due to database error: '. $e->getMessage());
		}//end

		return $result;

	}//end getTermLinkByTermids()


	/**
	* Returns the assetid of the thesaurus term referenced by the passed term name
	*
	* @param string	$name	The name of the sought term
	*
	* @return int
	* @access public
	*/
	function getTermIdByName($name)
	{
		$db = MatrixDAL::getDb();

		$name = strtolower($name);

		$sql = '
			SELECT
				termid
			FROM
				sq_thes_term
			WHERE
				lower(term) = :term
				AND
				thesid = :thesid
		';

		try {
			$query = MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'term', $name);
			MatrixDAL::bindValueToPdo($query, 'thesid', $this->id);
			$result = MatrixDAL::executePdoOne($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get thesaurus term id by name due to database error: '. $e->getMessage());
		}//end

		if (empty($result)) $result = NULL;

		return $result;

	}//end getTermIdByName()


	/**
	* Returns the assetids of the thesaurus terms referenced by the passed term names, in the format:
	* <pre>
	* Array(
	*	'termid'	=> 'name',
	* )
	* </pre>
	*
	* NOTE: This does not chunk the query, may need to be modified to do so in the future
	*
	* @param array	$names	An array of sought terms
	*
	* @return int
	* @access public
	*/
	function getTermIdsByNames($names)
	{
		if (empty($names)) return Array();

		$db = MatrixDAL::getDb();

		$formatted_names = Array();
		foreach ($names as $name) {
			$formatted_names[] = strtolower($name);
		}

		$bind_vars = Array (
						'names'	=> $formatted_names,
						'theid'	=> $this->id,
					 );

		try {
			$result = MatrixDAL::executeAssoc('thesaurus', 'getTermIdByNames', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to get thesaurus term ids by names due to database error: '. $e->getMessage());
		}//end

		return $result;

	}//end getTermIdsByNames()


	/**
	* Check whether supplied string is a term in this thesaurus
	*
	* @param string	$term	term name
	* @param int	$termid term id
	*
	* @return boolean
	* @access public
	*/
	function isTerm($term=NULL, $termid=0)
	{
		if (is_null($term)) return FALSE;

		$db = MAtrixDAL::getDb();

		$sql = '
			SELECT
				termid
			FROM
				sq_thes_term
			WHERE
				lower(term) = :term
				AND
				thesid = :thesid
		';
		if ($termid != 0) {
			$sql .= 	' AND termid <> :termid';
		}

		$bind_vars = Array (
						'term'	=> strtolower($term),
						'thesid' => $this->id,
					 );

		if ($termid != 0) {
			$bind_vars['termid'] = $termid;
		}

		try {
			$query	= MatrixDAL::preparePdoQuery($sql);
			MatrixDAL::bindValueToPdo($query, 'term', strtolower($term));
			MatrixDAL::bindValueToPdo($query, 'thesid', $this->id);
			if ($termid != 0) {
				MatrixDAL::bindValueToPdo($query, 'termid', $termid);
			}
			$result = MatrixDAL::executePdoOne($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get thesaurus term id due to database error: '.$e->getMessage());
		}//end

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
		$db = MatrixDAL::getDb();

		$bind_vars = Array (
						'assetid' => $this->id,
					 );

		try {
			$results = MatrixDAL::executeAssoc('thesaurus', 'getActiveRelations', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to get thesaurus active relations due to database error: '. $e->getMessage());
		}//end

		if(!empty($results)) {
			$new_res = Array();
			foreach ($results as $result) {
				$new_res[$result['relid']] = $result['relation'];
			}//end foreach
			$results = $new_res;
		}//end if

		return $results;

	}//end getActiveRelations()


	/**
	* Erases the contents of a thesaurus
	*
	* @return boolean
	* @access public
	*/
	function erase()
	{

		if (!$this->_deleteStoredContents()) return FALSE;

		$this->setAttrValue('relations', Array());
		$this->setAttrValue('reflections', Array());

		$this->markContentsChanged();

		return TRUE;

	}//end erase()


	/**
	* Deletes the contents of the database for this thesaurus
	*
	* @return boolean
	* @access private
	*/
	function _deleteStoredContents()
	{

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$db = MatrixDAL::getDb();

		$bind_vars = Array (
						'assetid'	=> $this->id,
					 );

		try {
			$result = MatrixDAL::executeQuery('thesaurus', 'deleteAllTerms', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete all thesaurus terms due to database error: '. $e->getMessage());
		}//end

		try {
			$result = MatrixDAL::executeQuery('thesaurus', 'deleteAllLinks', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete all thesaurus links due to database error: '. $e->getMessage());
		}//end

		try {
			$result = MatrixDAL::executeQuery('thesaurus', 'deleteAllRelations', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete all thesaurus relations due to database error: '. $e->getMessage());
		}//end

		$bind_vars['assetid'] = $this->id.':%';

		try {
			$result = MatrixDAL::executeQuery('thesaurus', 'deleteAllShadowLinks', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete all shadow thesaurus links due to database error: '. $e->getMessage());
		}//end

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

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
	* @param boolean	$short_name	Return the short_name or name?
	* @param int		$contextid	What context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Get a list of Terms that only act as parents in a relation
	*
	* @param int	$relation_id	relation Id
	*
	* @return array
	* @access public
	*/
	function getAbsoluteParentsInRelation($relation_id=NULL)
	{

		$db = MatrixDAL::getDb();

		if (!isset($relation_id)) return Array();

		$sql ='
			SELECT
				termid,
				term
			FROM
				sq_thes_term
			WHERE
				thesid = :thesid_1
				AND
				termid IN (
					SELECT DISTINCT
						major
					FROM
						sq_thes_lnk
					WHERE
						thesid = :thesid_2
						AND
						relid  = :relid_1
						AND
						major NOT IN (
							SELECT DISTINCT
								minor
							FROM
								sq_thes_lnk
							WHERE
								thesid = :thesid_3
								AND
								relid  = :relid_2
						)
				)
			ORDER BY
				term
		';

		$query = MatrixDAL::preparePdoQuery($sql);
		MatrixDAL::bindValueToPdo($query, 'thesid_1', $this->id);
		MatrixDAL::bindValueToPdo($query, 'thesid_2', $this->id);
		MatrixDAL::bindValueToPdo($query, 'thesid_3', $this->id);
		MatrixDAL::bindValueToPdo($query, 'relid_1', $relation_id);
		MatrixDAL::bindValueToPdo($query, 'relid_2', $relation_id);

		try {
			$result = MatrixDAL::executePdoAll($query);
		} catch (Exception $e) {
			throw new Exception('Unable to get absolute parent om relation due to database error: '. $e->getMessage());
		}//end

		return $result;

	}//end getAbsoluteParentsInRelation()


	/**
	* Get all lineages for a term
	*
	* @param string	$termid	the name of the term
	*
	* @return array
	* @access public
	*/
	function getLineagesForTerm($termid)
	{
		$result = Array();

		// this function is empty because lineages are hard to define and lookup for the general thesaurus
		// the only lineage that makes sense is the one using the hierarchy relations

		return $result;

	}//end getLineagesForTerm()


//--        BRIDGE FUNCTIONS        --//


	/**
	* Returns a reference to the asset represented by the passed assetid or NULL on error
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
	function getAsset($shadowid, $type_code='', $mute_errors=FALSE)
	{
		$asset = NULL;

		$id_parts = explode(':', $shadowid);
		if (!empty($id_parts[1])) {
			$id = $id_parts[1];

			$term = $this->getTermById($id);
			if (!empty($term)) {
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('thesaurus_term');
				$asset = new Thesaurus_Term($shadowid);
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
		return $this->_getTopTermsAssetMapLinks();

	}//end getAssetMapLinks()


	/**
	* Used by asset map to return a asset map-style link array
	*
	* @param int	$parent_term_id	The ID of the parent term
	*
	* @return array
	* @access public
	*/
	function _getShadowAssetMapLinks($parent_term_id=NULL)
	{
		$bridge_id = $this->id;

		$majorid = $bridge_id;
		if (!is_null($parent_term_id)) {
			$majorid .= ':'.$parent_term_id;
		}


		$active_relations = $this->getActiveRelations();
		$linked_terms = $this->getChildTerms($parent_term_id);

		$links = Array();

		foreach ($linked_terms as $term_link) {
			$link = Array();

			// mould it all to the asset map's liking
			$link['url'] = '';
			$link['path'] = '';
			$link['num_kids'] = -1;
			$link['accessible'] = 1;

			$link['majorid'] = $majorid;
			$link['minorid'] = $bridge_id.':'.$term_link['termid'];
			$link['assetid'] = $link['minorid'];
			$link['type_code'] = 'thesaurus_term';
			$link['minor_type_code'] = $link['type_code'];

			$link['linkid'] = $bridge_id.':'.$term_link['linkid'];
			// make name and short name the same
			$rel_name = array_get_index($active_relations, $term_link['relid']);
			$name_prefix = (empty($rel_name) ? '' : '('.$rel_name.') ');
			$link['name'] = $name_prefix.$term_link['term'];
			$link['short_name'] = $link['name'];

			$link['status'] = $this->status;
			$link['link_type'] = SQ_LINK_TYPE_1;
			$link['sort_order'] = 1;
			$link['is_dependant'] = 0;

			$links[] = $link;
		}

		return $links;

	}//end _getShadowAssetMapLinks()


	/**
	* Get all terms for display as top level terms in the asset map
	*
	* @return array
	* @access public
	*/
	function _getTopTermsAssetMapLinks()
	{
		$terms = Array();

		$linkid_prefix = '';
		if ($this->attr('hierarchy_mode')) {
			$linkid_prefix = ':hierarchy';
			$terms = $this->getAbsoluteParentsInRelation($this->attr('hierarchy_rel'));
		} else {
			$linkid_prefix = ':atoz';
			$terms = $this->getAllTerms();
		}

		$bridge_id = $this->id;


		$links = Array();

		foreach ($terms as $one_term) {
			$link = Array();

			// mould it all to the asset map's liking
			$link['url'] = '';
			$link['path'] = '';
			$link['num_kids'] = -1;
			$link['accessible'] = 1;

			$link['majorid'] = $bridge_id;
			$link['minorid'] = $bridge_id.':'.$one_term['termid'];
			$link['assetid'] = $link['minorid'];
			$link['type_code'] = 'thesaurus_term';
			$link['minor_type_code'] = $link['type_code'];

			$link['linkid'] = $bridge_id.$linkid_prefix.':'.$one_term['termid'];

			// make name and short name the same
			$link['name'] = $one_term['term'];
			$link['short_name'] = $link['name'];

			$link['status'] = $this->status;
			$link['link_type'] = SQ_LINK_TYPE_2;
			$link['sort_order'] = 1;
			$link['is_dependant'] = 0;

			$links[] = $link;

		}//end foreach

		return $links;

	}//end _getTopTermsAssetMapLinks()


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
	function getAssetInfo($assetids, $type_code=Array(), $strict_type_code=TRUE, $field='')
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
		$asset_info['assetid']		= $assetid;

		$id_parts = explode(':', $assetid);
		if (empty($id_parts[1])) return $asset_info;

		$term = $this->getTermById($id_parts[1]);
		if (empty($term)) return $asset_info;

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
	function getLinks($assetid, $link_types, $type_code='', $strict_type_code=TRUE, $side_of_link='major', $sort_by=NULL)
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
	*										this (if not NULL)
	*
	* @return array
	* @access public
	*/
	function getLink($assetid, $link_type=NULL, $type_code='', $strict_type_code=TRUE, $value=NULL, $side_of_link='major', $exclusive=NULL)
	{
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
	*									this (if not NULL)
	* @param boolean	$exclusive		The exclusive status for all the links must be
	*									this (if not NULL)
	*
	* @return array
	* @access public
	*/
	function getLinkByAsset($assetid, $other_assetid, $link_types=NULL, $value=NULL, $side_of_link='major', $force_array=FALSE, $dependant=NULL, $exclusive=NULL)
	{
		assert_valid_assetid($assetid);
		assert_valid_assetid($other_assetid);
		assert_false($side_of_link != 'major' && $side_of_link != 'minor', 'Unknown Side of Link "'.$side_of_link.'"');

		if ($side_of_link == 'major') {
			$major_assetid = $assetid;
			$minor_assetid = $other_assetid;
		} else {
			$major_assetid = $other_assetid;
			$minor_assetid = $assetid;
		}

		$major_parts = explode(':', $major_assetid);
		$majorid = array_get_index($major_parts, 1, NULL);

		$minor_parts = explode(':', $minor_assetid);
		$minorid = array_get_index($minor_parts, 1, NULL);
		if (empty($minorid)) return Array();

		$link_info = $this->getTermLinkByTermids($minorid, $majorid, $value);

		if (empty($link_info)) return Array();

		// construct template
		$link = Array(
					'majorid'			=> $major_assetid,
					'minorid'			=> $minor_assetid,
					'link_type'			=> SQ_LINK_TYPE_2,
					'major_type_code'	=> '',
					'minor_type_code'	=> 'thesaurus_term',
					'value'				=> $value,
					'linkid'			=> $this->id.':'.$link_info['linkid'],
					'is_dependant'		=> 0,
					'is_exclusive'		=> 0,
					'locked'			=> 0,
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

		$majorid = $this->id;

		if (isset($link_parts[2])) {
			// the major is the thesaurus
			$term_id = array_get_index($link_parts, 2);
			$minorid = $term_id;
			$value = NULL;
			$major_type_code = 'thesaurus';

		} else {
			$term_linkid = array_get_index($link_parts, 1);
			if (is_null($term_linkid)) {
				trigger_error('Invalid Link ID', E_USER_ERROR);
			}
			$link_info = $this->getTermLinkById($term_linkid);
			$minorid = array_get_index($link_info, 'minorid');
			$majorid .= ':'.array_get_index($link_info, 'major');
			$value = array_get_index($link_info, 'relation');
			$major_type_code = 'thesaurus_term';
		}

		// construct template
		$link = Array(
					'majorid'			=> $majorid,
					'minorid'			=> $this->id.':'.$minorid,
					'link_type'			=> SQ_LINK_TYPE_2,
					'major_type_code'	=> $major_type_code,
					'minor_type_code'	=> 'thesaurus_term',
					'value'				=> $value,
					'linkid'			=> $linkid,
					'is_dependant'		=> 0,
					'is_exclusive'		=> 0,
					'sort_order'		=> NULL,
					'locked'			=> 0,
				);

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
	*   exclusive   = TRUE|FALSE
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(
									'thesaurus_term'	=> Array(
															'card'		=> 'M',
															'exclusive'	=> FALSE,
														   ),
								   ),
				SQ_LINK_TYPE_2	=> Array(
									'thesaurus_term'	=> Array(
															'card'		=> 'M',
															'exclusive'	=> FALSE,
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
	* @param boolean	$dependant			if TRUE, results will be filtered to assets that are dependants.
	*										If FALSE, results will all not be dependants.
	*										If NULL, results will not be filtered
	* @param string		$sort_by			a field in the asset table you would like the results
	*										sorted by (eg. name, short_name etc)
	*
	* @return array
	* @access public
	* @see Asset_Manager::getChildren()
	*/
	function getChildren($assetid, $type_code='thesaurus_term', $strict_type_code=TRUE, $dependant=TRUE, $sort_by=NULL)
	{
		$result = Array();
		$id_parts = explode(':', $assetid);
		if ($type_code == 'thesaurus_term' || (is_array($type_code) && in_array('thesaurus_term', $type_code))) {
			// will return all terms, regardless of what param you pass in
			$terms = Array();
			if (isset($id_parts[1]) && $id_parts[0] == $this->id) {
				// if we are only looking for children terms underneath a term
				$terms = $this->getChildTerms($id_parts[1]);
			} else {
				$terms = $this->getAllTerms();
			}

			foreach ($terms as $term_info) {
				$result[$this->id.':'.$term_info['termid']] = Array(Array('type_code' => 'thesaurus_term'));
			}
		}

		return $result;

	}//end getChildren()


	/**
	* Get all asset ids that are above the passed assetid in the various trees in which it exists
	*
	* It returns Array(int => string)	returns an array of assetids and their type code
	*
	* @param int			$assetid			the id of the asset to get its parents for
	* @param string|array	$type_code			the type of asset that is linked (eg 'Page', 'File', etc)
	*											if an array returns link if matches any of the array values
	* @param boolean		$strict_type_code	whether we are finding assets that are just a $type_code
	*											or $type_code and any of it's sub-classes
	*
	* @return array
	* @access public
	*/
	function getParents($assetid, $type_code='', $strict_type_code=TRUE)
	{
		if (strpos($assetid, ':') !== FALSE) {
			list($realid, $shadowid) = explode(':', $assetid);
			$parents = Array($realid => $this->type());
			$parents += $GLOBALS['SQ_SYSTEM']->am->getParents($realid, $type_code, $strict_type_code);
			return $parents;
		}

		return Array();

	}//end getParents()


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
	*								if NULL or less than zero places at end of list
	* @param string		$dependant	'0' / '1' - whether the minor asset is dependant on the major
	* @param string		$exclusive	'0' / '1' - whether the major asset is to be the minor's only parent
	* @param boolean	$moving		whether or not this asset is being moved
	*
	* @return string
	* @access public
	*/
	function createAssetLink(&$major, &$minor, $link_type, $value='', $sort_order=NULL, $dependant='0', $exclusive='0', $moving=FALSE)
	{
		if ($major->id == $this->id) {
			// cannot link a term to a thesaurus
			// terms can only be linked to themselves
			// this term already exists; its appearance as the top term is controlled by the 'hierarchy' set of options
			trigger_error('Cannot link term to thesaurus. Term already exists in thesaurus. Its appearance as a top-level term is governed by the HIERARCHY options of the thesaurus and cannot be forced.', E_USER_ERROR);
			return 0;
		}

		$major_parts = explode(':',$major->id);
		$majorid = array_get_index($major_parts, 1);

		$minor_parts = explode(':',$minor->id);
		$minorid = array_get_index($minor_parts, 1);
		if (empty($minorid)) return NULL;

		$relid = empty($value) ? NULL : $value;
		$minor_term = NULL;
		$major_term = NULL;

		// can only link terms which belong to this thesaurus
		if (($minor_parts[0] != $this->id) || ($major_parts[0] != $this->id)) {
			return 0;
		}

		$minor_term = $minor->name;

		if (isset($major_parts[1])) {
			$major_term = $major->name;
		}

		if ($moving) {
			$link_info = $this->getTermLinkByTermids($minorid, $majorid, $relid);
			if (!is_null($link_info)) {
				trigger_localised_error('CORE0240');
				return FALSE;
			}
		} else {
			// if it's a new link, we use the default relation
			$relid = $this->attr('default_rel');
		}

		$term_linkid = $this->linkTerms($minorid, $majorid, $relid);

		if (is_null($term_linkid)) return FALSE;

		return $this->id.':'.$term_linkid;

	}//end createAssetLink()


	/**
	* Returns an array of all user or user_group assets that have the passed permission for the passed asset
	*
	* @param int		$assetid		the assetid for asset whose permissions to get
	* @param string		$permission		the permission code you are getting
	* @param boolean	$granted		type of Access : NULL = all, TRUE = granted, FALSE = denied
	* @param boolean	$and_greater	get effective permission (eg read access = read
	*									or write or admin)
	* @param boolean	$expand_groups	expand user groups so only user ids get returned
	*									(NOTE: only valid if $all_info is FALSE)
	* @param boolean	$all_info		when FALSE fn just returns an array of userids for
	*									those that have permission
	*									When TRUE fn returns all info about the permission in the form of
	*									Array(userid => granted)
	*									(NOTE: TRUE is only valid if $and_greater and
	*									$expand_groups are FALSE)
	*
	* @return array
	* @access public
	*/
	function getPermission($assetid, $permission, $granted=NULL, $and_greater=TRUE, $expand_groups=FALSE, $all_info=FALSE)
	{
		// All thesaurus terms inherit permissions of thesaurus.
		return $GLOBALS['SQ_SYSTEM']->am->getPermission($this->id, $permission, $granted, $and_greater, $expand_groups, $all_info);

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
		$this->setAttrValue('contents_changed',TRUE);
		$this->saveAttributes();
		$GLOBALS['SQ_SYSTEM']->restoreRunLevel();

		return;

	}//end markContentsChanged()


	/**
	* Return the number of links in the system that involve the specified asset
	*
	* Can be restricted by either link type and/or asset type code
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
	function countLinks($assetid, $side_of_link='major', $link_types=0, $type_code='', $strict_type_code=TRUE, $ignore_linkid=0)
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
	function deleteAssetLink($linkid, $moving=FALSE)
	{
		$link_parts = explode(':', $linkid);

		if (isset($link_parts[1])) {
			if (isset($link_parts[2])) {
				if ($link_parts[1] == 'atoz') {
					// delete term (in A-to-Z mode)
					$this->deleteTerm($link_parts[2]);
				}
			} else {
				$this->deleteTermLink($link_parts[1]);
			}
		}
		return TRUE;

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
		if (empty($linkid)) return FALSE;

		// reflection support
		$link_info = $this->getTermLinkById($linkid);
		$major = $link_info['major'];
		$minor = $link_info['minor'];
		$relid = $link_info['relid'];

		$this->_deleteTermLink($linkid);

		if ($this->attr('enforce_reflection')) {
			$ref_relid = $this->getReflectionForRelation($relid);
			if (!is_null($ref_relid)) {
				$reflect_link = $this->getTermLinkByTermids($major, $minor, $ref_relid);
				if (!empty($reflect_link)) {
					$this->_deleteTermLink($reflect_link['linkid']);
				}
			}
		}
		return TRUE;

	}//end deleteTermLink()


	/**
	* Delete a link between two terms
	*
	* @param int	$linkid	link id to remove
	*
	* @return void
	* @access public
	*/
	function _deleteTermLink($linkid)
	{
		if (empty($linkid)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		try {
			$bind_vars	= Array (
							'linkid'	=> $linkid,
							'assetid'	=> $this->id,
						  );
			$result = MatrixDAL::executeQuery('thesaurus', 'deleteLink', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete thesaurus term link due to the following database error:'.$e->getMessage());
		}//end try catch

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end _deleteTermLink()


	/**
	* Delete a term
	*
	* @param int	$termid	term id to remove
	*
	* @return void
	* @access public
	*/
	function deleteTerm($termid)
	{
		if (empty($termid)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// delete the term
		try {
			$bind_vars	= Array (
							'assetid'	=> $this->id,
							'termid'	=> $termid,
						  );
			$result	= MatrixDAL::executeQuery('thesaurus', 'deleteTerm', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete thesaurus term due to the following database error:'.$e->getMessage());
		}//end try catch

		try {
			$bind_vars	= Array (
							'termid'	=> $termid,
							'assetid'	=> $this->id,
						  );
			$result	= MatrixDAL::executeQuery('thesaurus', 'deleteThesaurusLinksContainingTerm', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete thesaurus links containing term due to the following database error:'.$e->getMessage());
		}//end try catch

		try {
			$bind_vars	= Array (
							'assetid'	=> $this->id.':'.$termid,
						  );
			$result	= MatrixDAL::executeQuery('thesaurus', 'deleteAllShadowLinks', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete shadow links due to the following database error:'.$e->getMessage());
		}//end try catch

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end deleteTerm()


	/**
	* Delete a link between two terms
	*
	* @param string	$relid	link id to remove
	*
	* @return void
	* @access public
	*/
	function deleteRelation($relid)
	{
		if (empty($relid)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// delete the term
		try {
			$bind_vars	= Array (
							'relation'	=> $relid,
							'assetid'	=> $this->id,
						  );
			$result	= MatrixDAL::executeQuery('thesaurus', 'deleteRelation', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete thesaurus terms relation due to the following database error:'.$e->getMessage());
		}//end try catch

		try {
			$bind_vars	= Array (
							'relation'	=> $relid,
							'assetid'	=> $this->id,
						  );
			$result	= MatrixDAL::executeQuery('thesaurus', 'deleteLinksContainingRelation', $bind_vars);
		} catch (Exception $e) {
			throw new Exception('Unable to delete links containing relation due to the following database error:'.$e->getMessage());
		}//end try catch

		// cleanup the reflections
		// NOTE: In the future it might be necessary to wipe the reflective relation as well - Reflection Integrity
		$this->deleteReflection($relid);

		// make sure any settings we have do not use this relation id
		$synonym_rel = $this->attr('synonym_rel');
		if ($synonym_rel == $relid) {
			$this->setAttrValue('synonym_rel', NULL);
		}

		$abbreviation_rel = $this->attr('abbreviation_rel');
		if ($abbreviation_rel == $relid) {
			$this->setAttrValue('abbreviation_rel', NULL);
		}

		$default_rel = $this->attr('default_rel');
		if ($default_rel == $relid) {
			$this->setAttrValue('default_rel', NULL);
		}


		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();


		return TRUE;

	}//end deleteRelation()


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
	function updateLink($linkid, $link_type=NULL, $value=NULL, $new_sort_order=NULL)
	{
		$linkid_parts = explode(':', $linkid);
		if (!isset($linkid_parts[1]) || ($linkid_parts[0] != $this->id)) {
			return FALSE;
		}

		return TRUE;

	}//end updateLink()


	/**
	* Determine if the passed assetid or assetids refer(s) to a valid asset in the system
	*
	* @param int|array	$assetids	the ID of the asset we are going to the check for existence
	*								or an array of assetids we want to check
	*
	* @return mixed boolean|array
	* @access public
	*/
	function assetExists($assetids)
	{

		$shadows = Array();
		$shadow_asset_ids = Array();

		// number of assetids
		$count = count($assetids);

		$is_array = is_array($assetids);
		$is_empty = empty($assetids);

		// assert assetid format
		if ($is_array && !$is_empty) {
			foreach ($assetids as $key => $value) {
				assert_valid_assetid($value);
			}
		} else if (!$is_empty) {
			assert_valid_assetid($assetids);
		}

		$db = MatrixDAL::getDb();
		$sql = 'SELECT termid, thesid
				FROM '.SQ_TABLE_RUNNING_PREFIX.'thes_term ';

		// single asset id
		if (!$is_array) {
			if ($is_empty) return FALSE;
			$id_parts = explode(':', $assetids);

			$where = 'termid = :termid AND thesid = :thesid';
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$query = MatrixDAL::preparePdoQuery($sql.$where);
			MatrixDAL::bindValueToPdo($query, 'thesid', $id_parts[0]);
			MatrixDAL::bindValueToPdo($query, 'termid', $id_parts[1]);
			$db_assetids = MatrixDAL::executePdoOne($query);

			return ($db_assetids == $assetids);
		}

		// array of asset ids
		$existing_asset_ids = Array();
		if (!$is_empty) {
			$in = 'termid IN (';
			$thesid_array = Array();
			for ($i=0; $i<count($assetids); $i++) {
				$id_parts = explode(':', $assetids[$i]);
				// build an array of thesids for each termid
				$thesid_array[$id_parts[1]] = $id_parts[0];
				$in .= MatrixDAL::quote($id_parts[1]).(($i == count($assetids) - 1) ? ')' : ',');
			}

			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($in);
			$db_assetids = MatrixDAL::executeSqlAll($sql.$where);

			foreach ($db_assetids as $db_id) {
				if ($db_id['thesid'] == $thesid_array[$db_id['termid']]) {
					$existing_asset_ids[] = $db_id['thesid'].':'.$db_id['termid'];
				}
			}
		}

		return $existing_asset_ids;

	}//end assetExists()


	/**
	* Get the reflection array
	*
	* @return void
	* @access public
	*/
	function getReflections()
	{
		return $this->attr('reflections');

	}//end getReflections()


	/**
	* Get reflection relation id given the other relation
	*
	* @param string	$start	id of one relation in a reflection
	*
	* @return string
	* @access public
	*/
	function getReflectionForRelation($start)
	{
		$reflections = $this->getReflections();
		return array_get_index($reflections, $start);

	}//end getReflectionForRelation()


	/**
	* Add a reflection
	*
	* @param string	$start	id of one relation in a reflection
	* @param string	$end	id of another relation in a reflection
	*
	* @return void
	* @access public
	*/
	function addReflection($start, $end)
	{
		$reflections = $this->getReflections();

		if (isset($relfections[$start]) || isset($reflections[$end])) {
			return FALSE;
		}

		$reflections[$start] = $end;
		$reflections[$end] = $start;

		$this->setReflections($reflections);

	}//end addReflection()


	/**
	* Delete a reflection
	*
	* @param string	$start	id of one relation in a reflection
	*
	* @return void
	* @access public
	*/
	function deleteReflection($start)
	{
		$reflections = $this->getReflections();

		$end = array_get_index($reflections, $start);
		if (is_null($end)) return FALSE;

		unset($reflections[$start]);
		unset($reflections[$end]);

		$this->setReflections($reflections);

	}//end deleteReflection()


	/**
	* Set an array of reflections
	*
	* @param array	$ref_array	array of reflections in format 'a'=>'b', 'b'=>'a'
	*
	* @return void
	* @access public
	*/
	function setReflections($ref_array)
	{
		$this->setAttrValue('reflections', $ref_array);
		return TRUE;

	}//end setReflections()


	/**
	* Returns TRUE if this type of asset can be cloned
	*
	* Override this function if you do not want a particular type of asset to be cloned
	*
	* @return boolean
	* @access public
	*/
	function canClone()
	{
		return FALSE;

	}//end canClone()


	/**
	* Returns an array of assetid's + info in the order that they propogate out from THIS BRIDGE'S url
	*
	* @param string	$assetid	the id of the last asset in the lineage
	* @param string	$protocol	the protocol to match -> null means it is ignored
	* @param string	$url		the url to check for -> null defaults it to current url
	*
	* @return array
	* @access public
	*/
	function getLineageFromURL($assetid, $protocol, $url)
	{
		return Array();

	}//end getLineageFromURL()


}//end class
?>
