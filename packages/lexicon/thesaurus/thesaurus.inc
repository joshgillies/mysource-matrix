<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: thesaurus.inc,v 1.12 2005/04/29 05:39:50 gsherwood Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Thesaurus
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Dmitry Baranovskiy <dbaranovskiy@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage lexicon
*/
class Thesaurus extends Asset
{

	var $BAD_ID = -1;

	var $REL_ABBREVIATION = 'Abbreviation';
	var $REL_CATEGORY     = 'Category';


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
	* @access public
	* @return boolean
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
	* @access private
	* @return boolean
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return false;
		return create_directory($this->data_path);

	}//end _createAdditional()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Finish Import
	*
	* After XML file has been imported, checks for built-in relations and enables them
	*
	* @access public
	* @return void
	*/
	function finishImport()
	{
		$builtin_relations = $relation_settings = Array();

		// ABBREVIATION support
		$rel = $this->getRelationByName($this->REL_ABBREVIATION);
		if (!empty($rel)) {
			$builtin_relations[$this->REL_ABBREVIATION] = $rel['id'];
		}

		// CATEGORY support
		$rel = $this->getRelationByName($this->REL_CATEGORY);
		if (!empty($rel)) {
			// get top categories
			$top_categories = $this->getAbsoluteParentsInRelation($rel['id']);
			$relation_settings[$this->REL_CATEGORY]['top_categories'] = $top_categories;

			$builtin_relations[$this->REL_CATEGORY] = $rel['id'];
		}

		$this->setAttrValue('builtin_relations', $builtin_relations);
		$this->setAttrValue('relation_settings', $relation_settings);
		$this->setAttrValue('active', true);
		$this->saveAttributes();

	}//end finishImport()


	/**
	* Check if relation is enabled
	*
	* @param string rel_name    relation name
	* @access public
	* @return boolean
	*/
	function isRelationEnabled($rel_name)
	{
		$builtin_relations = $this->attr('builtin_relations');

		if (!$this->attr('active')) {
			return false;
		}

		return array_get_index($builtin_relations, $rel_name, false);

	}//end isRelationEnabled()


//--        ABBREVIATIONS        --//


	/**
	* Get an array of abbrevations
	*
	* the result is an array of format ("A" => "Australia")
	* abbreviation => expansion
	*
	* NOTICE: An expansion can have only one abbreviation
	*
	* @access public
	* @return array
	*/
	function getAbbreviations()
	{
		$result = Array();
		$rel_id = $this->isRelationEnabled($this->REL_ABBREVIATION);

		if ($rel_id === false) {
			return $result;
		}

		$abbrev_entities = $this->getEntitiesInRelation($rel_id);
		foreach ($abbrev_entities as $entity_pair) {
			$result[$entity_pair['child']['name']] = $entity_pair['parent']['name'];
		}

		return $result;

	}//end getAbbreviations()


//--        CATEGORIES        --//


	/**
	* Get Top Categories
	*
	* Given that CATEGORY relation is enabled, this returns the root categories
	*
	* @access public
	* @return array
	*/
	function getTopCategories()
	{
		$rel_id = $this->isRelationEnabled($this->REL_CATEGORY);

		if ($rel_id === false) {
			return Array();
		}

		$relation_settings = $this->attr('relation_settings');

		return $relation_settings[$this->REL_CATEGORY]['top_categories'];

	}//end getTopCategories()


	/**
	* Get Subcategories
	*
	* Obtain the subcategories for an entity
	*
	* @param int entity_id      Id of entity under question
	* @access public
	* @return array
	*/
	function getSubcategories($entity_id)
	{
		$rel_id = $this->isRelationEnabled($this->REL_CATEGORY);

		if ($rel_id === false) {
			return Array();
		}

		$subcategories = $this->getChildEntitiesInRelation($rel_id, $entity_id);

		return $subcategories;

	}//end getSubcategories()


	/**
	* Check if an entity is a category
	*
	* @param int entity_id      Id of entity under question
	* @access public
	* @return boolean
	*/
	function isCategory($entity_id)
	{
		$rel_id = $this->isRelationEnabled($this->REL_CATEGORY);

		if ($rel_id === false) {
			return 0;
		}

		return (bool) $this->countChildEntitiesInRelation($rel_id, $entity_id);

	}//end isCategory()


//--        CORE THESAURUS RELATIONAL ENGINE        --//


	/**
	* Get All Entities in this thesaurus
	*
	* @access public
	* @return array
	*/
	function getAllEntities()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				id, name, description
			FROM
				sq_lex_thes_ent
			WHERE
				thes_id = '.$db->quoteSmart($this->id);

		$result = $db->getAll($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getAllEntities()


	/**
	* Get a count of all entities
	*
	* @access public
	* @return integer
	*/
	function countAllEntities()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				count(*)
			FROM
				sq_lex_thes_ent
			WHERE
				thes_id = '.$db->quoteSmart($this->id);

		$result = $db->getOne($sql);

		assert_valid_db_result($result);
		return $result;

	}//end countAllEntities()


	/**
	* Search entities by name
	*
	* @param string guess   string to search by
	* @access public
	* @return array
	*/
	function searchEntities($guess='')
	{
		if (empty($guess)) {
			return Array();
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				id, name, description
			FROM
				sq_lex_thes_ent
			WHERE
					thes_id = '.$db->quoteSmart($this->id).'
				AND
					name ILIKE \'%'.$db->escapeSimple($guess).'%\'
			';

		$result = $db->getAll($sql);
		assert_valid_db_result($result);
		return $result;

	}//end searchEntities()


	/**
	* Get Entity By Id
	*
	* @param integer entity_id   id we are looking for
	* @access public
	* @return array
	*/
	function getEntityById($entity_id=false)
	{
		if (!$entity_id) {
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				id, name, description
			FROM
				sq_lex_thes_ent
			WHERE
				thes_id = '.$db->quoteSmart($this->id).'
				AND id = '.$db->quoteSmart($entity_id);

		$result = $db->getRow($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getEntityById()


	/**
	* Get Entity By Exact Name
	*
	* @param string name    exact entity name
	* @access public
	* @return array
	*/
	function getEntityByName($name='')
	{
		if (empty($name)) {
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				id, name, description
			FROM
				sq_lex_thes_ent
			WHERE
				thes_id = '.$db->quoteSmart($this->id).'
				AND name = '.$db->quoteSmart($name);

		$result = $db->getRow($sql);
		assert_valid_db_result($result);
		return $result;

	}//end getEntityByName()


	/**
	* Get All the child entities in a relation, possibly limited by parent entity id
	*
	* @param integer relation_id    Relation Id
	* @param integer $parent_id     Parent Entity Id (optional)
	* @access public
	* @return array    array of entity arrays
	*/
	function getChildEntitiesInRelation($relation_id=null, $parent_id=null)
	{

		if (!isset($relation_id)) {
			return null;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT DISTINCT
				e.id,
				e.name,
				e.description
			FROM
				sq_lex_thes_ent_rel er,
				sq_lex_thes_ent e
			WHERE
					er.thes_id  = '.$db->quoteSmart($this->id).'
				AND
					er.rel_id   = '.$db->quoteSmart($relation_id).'
				AND
					e.thes_id   = er.thes_id
				AND
					e.id         = er.cld_ent_id
		';

		// restrict the result to only the children of a given parent
		if (!empty($parent_id)) {
			$sql .= ' AND er.pnt_ent_id = '.$db->quoteSmart($parent_id);
		}

		$result = $db->getAll($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getChildEntitiesInRelation()


	/**
	* count child entities in a relation
	*
	* Parent ID is optional, but without it, the sence that this function makes is limited
	*
	* @param integer $relation_id    Relation Id
	* @param integer $parent_id      Parent Id (optional)
	* @access public
	* @return integer
	*/
	function countChildEntitiesInRelation($relation_id=null, $parent_id=snull)
	{

		if (!isset($relation_id)) {
			return null;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				COUNT(*)
			FROM
				sq_lex_thes_ent_rel
			WHERE
					thes_id  = '.$db->quoteSmart($this->id).'
				AND
					rel_id   = '.$db->quoteSmart($relation_id);

		// restrict the result to only the children of a given parent
		if (!empty($parent_id)) {
			$sql .= ' AND pnt_ent_id = '.$db->quoteSmart($parent_id);
		}

		$result = $db->getOne($sql);

		assert_valid_db_result($result);
		return $result;

	}//end countChildEntitiesInRelation()


	/**
	* Get Parent Entities in Relation
	*
	* Returns an array of all the entities that act as a parent in a certain relationship
	* If Child id is supplied, the listing is limited by the parents of that child only
	*
	* @param integer relation_id    Relation Id
	* @param integer $parent_id     Parent Entity Id (optional)
	* @access public
	* @return array    array of entity arrays
	*/
	function getParentEntitiesInRelation($relation_id=null, $child_id=null)
	{

		if (!isset($relation_id)) {
			return null;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql ='

			SELECT DISTINCT
				e.id,
				e.name,
				e.description
			FROM
				sq_lex_thes_ent_rel er,
				sq_lex_thes_ent e
			WHERE
					er.thes_id  = '.$db->quoteSmart($this->id).'
				AND
					er.rel_id   = '.$db->quoteSmart($relation_id).'
				AND
					e.thes_id   = er.thes_id
				AND
					e.id        = er.pnt_ent_id

		';

		// restrict the result to only the parents of a given child
		if (!empty($child_id)) {
			$sql .= ' AND er.cld_ent_id = '.$db->quoteSmart($child_id);
		}

		$result = $db->getAll($sql, $data);

		assert_valid_db_result($result);
		return $result;

	}//end getParentEntitiesInRelation()


	/**
	* Get the Relations in which a given entity acts as a child
	*
	* @param integer $child_entity_id   entity id
	* @access public
	* @return array
	*/
	function getParentRelationsForEntity($child_entity_id=null)
	{

		if (!isset($child_entity_id)) {
			return null;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql ='

			SELECT DISTINCT
				r.id,
				r.name
			FROM
				sq_lex_thes_rel r,
				sq_lex_thes_ent_rel er
			WHERE
					r.thes_id     = '.$db->quoteSmart($this->id).'
				AND
					er.cld_ent_id = '.$db->quoteSmart($child_entity_id).'
				AND
					er.thes_id    = r.thes_id
				AND
					er.reln_id    = r.id
		';

		$result = $db->getAll($sql, $data);

		assert_valid_db_result($result);
		return $result;

	}//end getParentRelationsForEntity()


	/**
	* Count Parent Relations For Entity
	*
	* Get a count of how many relations a given asset is in acting as a child
	*
	* @param integer $child_entity_id   entity id
	* @access public
	* @return integer
	*/
	function countParentRelationsForEntity($child_entity_id=null)
	{

		if (!isset($child_entity_id)) {
			return null;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				COUNT(rel_id)
			FROM
				sq_lex_thes_ent_rel
			WHERE
					thes_id = '.$db->quoteSmart($this->id).'
				AND
					cld_ent_id = '.$db->quoteSmart($child_entity_id);

		$result = $db->getOne($sql);

		assert_valid_db_result($result);
		return $result;

	}//end countParentRelationsForEntity()


	/**
	* Cound Child Relations for Entity
	*
	* Get a count of how many relations a given asset is in acting as a parent
	*
	* @param integer $parent_entity_id   entity id
	* @access public
	* @return integer
	*/
	function countChildRelationsForEntity($parent_entity_id=null)
	{

		if (!isset($parent_entity_id)) {
			return null;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				count(rel_id)
			FROM
				sq_lex_thes_ent_rel
			WHERE
					thes_id = '.$db->quoteSmart($this->id).'
				AND
					pnt_ent_id = '.$db->quoteSmart($parent_entity_id);

		$result = $db->getOne($sql);

		assert_valid_db_result($result);
		return $result;

	}//end countChildRelationsForEntity()


	/**
	* Get Child Relations for Entity
	*
	* Get the relations a given asset is in acting as a parent
	*
	* @param integer $parent_entity_id   entity id
	* @access public
	* @return array    array of entity arrays
	*/
	function getChildRelationsForEntity($parent_entity_id=null)
	{

		if (!isset($parent_entity_id)) {
			return null;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql ='
			SELECT DISTINCT
				r.id,
				r.name
			FROM
				sq_lex_thes_rel r,
				sq_lex_thes_ent_rel er
			WHERE
					r.thes_id     = '.$db->quoteSmart($this->id).'
				AND
					er.pnt_ent_id = '.$db->quoteSmart($parent_entity_id).'
				AND
					er.thes_id    = r.thes_id
				AND
					er.rel_id     = r.id

		';

		$result = $db->getAll($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getChildRelationsForEntity()


	/**
	* Get All Relations
	*
	* @access public
	* @return array    array of all relations
	*/
	function getAllRelations()
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				id, name
			FROM
				sq_lex_thes_rel
			WHERE
				thes_id = '.$db->quoteSmart($this->id);

		$result = $db->getAll($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getAllRelations()


	/**
	* Count All Relations
	*
	* @access public
	* @return integer
	*/
	function countAllRelations()
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				count(*)
			FROM
				sq_lex_thes_rel
			WHERE
				thes_id = '.$db->quoteSmart($this->id);

		$result = $db->getOne($sql);

		assert_valid_db_result($result);
		return $result;

	}//end countAllRelations()


	/**
	* Get Relation By Id
	*
	* @param integer $id    relation ID
	* @access public
	* @return array    relation
	*/
	function getRelationById($id=null)
	{

		if (!isset($id)) {
			return null;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			SELECT
				id, name
			FROM
				sq_lex_thes_rel
			WHERE
					thes_id = '.$db->quoteSmart($this->id).'
				AND
					id = '.$db->quoteSmart($id);

		$result = $db->getRow($sql);

		assert_valid_db_result($result);
		return $result;

	}//end getRelationById()


	/**
	* Get Relation by Name
	*
	* @param string $name    relation name
	* @access public
	* @return array    relation
	*/
	function getRelationByName($name=null)
	{

		if (!isset($name)) {
			return null;
		}

		$relations = $this->attr('relations');
		$rel_id = array_get_index($relations, $name);

		if (!empty($rel_id)) {
			// first check if the relation exists in the relation cache
			$result['id']   = $rel_id;
			$result['name'] = $name;
		} else {
			// then check the database
			$db = &$GLOBALS['SQ_SYSTEM']->db;

			$sql = '
				SELECT
					id, name
				FROM
					sq_lex_thes_rel
				WHERE
						thes_id = '.$db->quoteSmart($this->id).'
					AND
						name = '.$db->quoteSmart($name);

			$result = $db->getRow($sql);

			assert_valid_db_result($result);

			// if relation exists, cache it
			if (!empty($result)) {
				$relations[$name] = $result['id'];
				$this->setAttrValue('relations',$relations);
			}
		}

		return $result;

	}//end getRelationByName()


	/**
	* Get All Entities in Relation
	*
	* Returns a possibly VERY large array of (parent,child) tuples
	*
	* @param integer $relation_id    relation Id
	* @access public
	* @return array
	*/
	function getEntitiesInRelation($relation_id=null)
	{

		if (!isset($relation_id)) {
			return null;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql ='

			SELECT
				e1.id,
				e1.name,
				e1.description,
				e2.id,
				e2.name,
				e2.description
			FROM
				sq_lex_thes_ent e1,
				sq_lex_thes_ent e2,
				sq_lex_thes_ent_rel er
			WHERE
					er.thes_id = '.$db->quoteSmart($this->id).'
				AND
					er.rel_id  = '.$db->quoteSmart($relation_id).'
				AND
					e1.id      = er.pnt_ent_id
				AND
					e2.id      = er.cld_ent_id
				AND
					e1.thes_id = er.thes_id
				AND
					e2.thes_id = er.thes_id

		';

		$db->setFetchMode(DB_FETCHMODE_ASSOC);
		$result = $db->getAll($sql);
		assert_valid_db_result($result);

		// assemble an array of entity tuples ((entity, entity),...)
		$relations = Array();
		foreach ($result as $row) {
			$e1 = $e2 = Array();
			list(
				$e1['id'], $e1['name'], $e1['description'],
				$e2['id'], $e2['name'], $e2['description']
			) = $row;

			$relations[] = Array('parent'=>$e1, 'child'=>$e2);
		}

		return $relations;

	}//end getEntitiesInRelation()


	/**
	* Add Entity To The Thesaurus
	*
	* @param string $name        Entity Name
	* @param string $description Entity Description (optional)
	*
	* @access public
	* @return integer    entity id
	*/
	function addEntity($name, $description='')
	{

		if (!isset($name)) {
			return $this->BAD_ID;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$id = $db->nextId('sq_lex_thes_ent_id');
		assert_valid_db_result($id);

		$sql = '
			INSERT INTO sq_lex_thes_ent
				(thes_id, id, name, description)
			VALUES
				('.$db->quoteSmart($this->id).','.$db->quoteSmart($id).','.$db->quoteSmart($name).','.$db->quoteSmart($description).')';

		$result = $db->query($sql);
		assert_valid_db_result($result);

		return $id;

	}//end addEntity()


	/**
	* Add relatin to this thesaurus
	*
	* @param string $name    relation name
	* @access public
	* @return integer    relation id
	*/
	function addRelation($name)
	{

		if (!isset($name)) {
			return $this->BAD_ID;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$id = $db->nextId('sq_lex_thes_rel_id');
		assert_valid_db_result($id);

		$sql = '
			INSERT INTO sq_lex_thes_rel
				(
					thes_id,
					id,
					name
				)
			VALUES
				('.
					$db->quoteSmart($this->id).','.
					$db->quoteSmart($id).','.
					$db->quoteSmart($name).'
				)
		';

		$result   = $db->query($sql);

		assert_valid_db_result($result);

		// cache relation
		$cached_relations = $this->attr('relations');
		$cached_relations[$name] = $id;
		$this->setAttrValue('relations', $cached_relations);

		return $id;

	}//end addRelation()


	/**
	* Connect two entities by a relation
	*
	* Relies on the database to enforce the fact that two entities can
	* only be linked once by a particular relationship in a particular thesaurus
	* Relationship is an arrow pointing from parent to child
	* A-x->B and B-x->A are different relationships, just like A-x->B and A-y->B
	*
	* @access public
	* @return boolean
	*/
	function connectEntities($parent_id, $child_id, $relation_id)
	{

		if (!isset($parent_id) || !isset($child_id) || !isset($relation_id)) {
			return false;
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql ='
			INSERT INTO sq_lex_thes_ent_rel
				(
					thes_id,
					pnt_ent_id,
					cld_ent_id,
					rel_id
				)
			VALUES
				('.
					$db->quoteSmart($this->id).','.
					$db->quoteSmart($parent_id).','.
					$db->quoteSmart($child_id).','.
					$db->quoteSmart($relation_id).'
				)
		';

		$result   = $db->query($sql);
		assert_valid_db_result($result);

		return true;

	}//end connectEntities()


	/**
	* Is Bad ID?
	*
	* Returns true if supplied ID is bad
	*
	* @param integer $id    entity id
	* @access public
	* @return boolean
	*/
	function isBadId($id)
	{
		return ($this->BAD_ID == $id);

	}//end isBadId()


	/**
	* Get a list of Entities that only act as parents in a relation
	*
	* @param integer $relation_id    relation Id
	* @access public
	* @return array    array of entities
	*/
	function getAbsoluteParentsInRelation($relation_id=null)
	{

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		if (!isset($relation_id)) {
			return Array();
		}

		$sql ='
			SELECT
				id,
				name,
				description
			FROM
				sq_lex_thes_ent
			WHERE
				id IN (
					SELECT DISTINCT
						pnt_ent_id
					FROM
						sq_lex_thes_ent_rel
					WHERE
							thes_id = '.$db->quoteSmart($this->id).'
						AND
							rel_id  = '.$db->quoteSmart($relation_id).'
						AND
							pnt_ent_id NOT IN (
								SELECT DISTINCT
									cld_ent_id
								FROM
									sq_lex_thes_ent_rel
								WHERE
										thes_id = '.$db->quoteSmart($this->id).'
									AND
										rel_id  = '.$db->quoteSmart($relation_id).'
						)
				)
		';

		$result = $db->getAll($sql);
		assert_valid_db_result($result);

		return $result;

	}//end getAbsoluteParentsInRelation()


	/**
	* Erases the contents of a thesaurus
	*
	* @access public
	* @return boolean
	*/
	function erase()
	{

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$this->setAttrValue('relations', Array());
		$this->setAttrValue('active', false);

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
	* NOTE: does not disable the thesaurus
	*
	* @access private
	* @return boolean
	*/
	function _deleteStoredContents()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			DELETE FROM sq_lex_thes_ent     WHERE thes_id = '.$db->quoteSmart($this->id).';
			DELETE FROM sq_lex_thes_rel     WHERE thes_id = '.$db->quoteSmart($this->id).';
			DELETE FROM sq_lex_thes_ent_rel WHERE thes_id = '.$db->quoteSmart($this->id).';
		';

		$result   = $db->query($sql);
		assert_valid_db_result($result);

		return true;

	}//end _deleteStoredContents()


	/**
	* Get the path to the imported xml file
	*
	* @access public
	* @return string
	*/
	function getImportPath()
	{
		return $this->data_path.'/import.xml';

	}//end getImportPath()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @access private
	* @return string
	*/
	function _getName()
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Check if thesaurus is active
	*
	* @access public
	* @return boolean
	*/
	function isActive()
	{
		return $this->attr('active');

	}//end isActive()


}//end class

?>