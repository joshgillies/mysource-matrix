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
* $Id: thesaurus.inc,v 1.4 2004/12/02 21:50:19 arailean Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* Thesaurus
*
* Purpose
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @author  Dmitry Baranovskiy <dbaranovskiy@squiz.net>
* @version $Revision: 1.4 $
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
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		if (!create_directory($this->data_path)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()



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


	//// ABBREVIATIONS ////
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

	//// finish ABBREVIATIONS ////


	//// CATEGORIES ////

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

	//// finish CATEGORIES ////


	/**
	* Get all abbreviation relationship instances in this thesaurus.
	*
	* THIS FUNCTION ONLY TO BE CALLED - AND ONLY TEMPORARILY - FROM THE MATRIX_EXPAND_KEYWORDS
	* WYSIWYG PLUGIN.  OR ELSE!
	*
	* @return array (abbrev => expansion)
	* @access public
	*/
	function getAbbrevsDeprecated()
	{
		return Array(
			'AAA' =>	'abdominal aortic aneurysm',
			'AAS' =>	'acute abdominal series',
			'ABD' =>	'abdomen',
			'ABG' =>	'arterial blood gas',
			'AC' =>		'before eating',
			'ACLS' =>	'advanced cardiac life support',
			'ACTH' =>	'adrenocorticotropic hormone',
			'ADH' =>	'anti-diuretic hormone',
			'ad lib' => 'as much as needed',
			'AF' =>		'atrial fibrillation',
			'AFB' =>	'acid-fast bacilli',
			'AFP' =>	'alpha-fetoprotein',
			'A/G' =>	'albumin/globulin ratio',
			'AI' =>		'aortic insufficiency',
			'AKA ' =>	'above the knee amputation',
			'ALL' =>	'acute lymphocytic leukemia',
			'amb' =>	'ambulate',
			'AML' =>	'acute myelogenous leukemia',
			'ANA' =>	'antinuclear antibody',
			'AOB' =>	'alcohol on breath',
			'AODM' =>	'adult onset diabetes mellitus',
			'AP' =>		'anteroposterior',
			'ARDS' => 	'acute respiratory distress syndrome',
			'ARF' =>	'acute renal failure',
			'AS' =>		'aortic stenosis',
			'ASAP' =>	'as soon as possible',
			'ASCVD' =>	'atherosclerotic cardiovascular disease',
			'ASD' =>	'atrial septal defect',
			'ASHD' =>	'atherosclerotic heart disease',
			'AV' =>		'atrioventricular',
			'A-V' =>	'arteriovenous',
			'A-VO2' =>	'arteriovenous oxygen',
			'BBB' =>	'bundle branch block',
			'BCAA' =>	'branched chain amino acids',
			'BE' =>		'barium enema',
			'BEE' =>	'basal energy expenditure',
			'bid' =>	'twice a day',
			'BKA' =>	'below the knee amputation',
			'BM' =>		'bowel movement',
			'BMR' =>	'basal metabolic rate',
			'BOM' =>	'bilateral otitis media',
			'BP' =>		'blood pressure',
			'BPH' =>	'benign prostatic hypertrophy',
			'BPM' =>	'beats per minute',
			'BRBPR' =>	'bright red blood per rectum',
			'BRP' =>	'bathroom priviledges',
			'BS' =>		'bowel',
			'BUN' =>	'blood urea nitrogen',
			'BW' =>		'body weight',
			'BX' =>		'with',
			'C&S' =>	'culture and sensitivity',
			'CA' =>		'cancer',
			'Ca' =>		'calcium',
			'CAA' =>	'crystalline amino acids',
			'CABG' =>	'coronary artery bypass graft',
			'CAD' =>	'coronary artery disease',
			'CAT' =>	'computerized axial tomography',
			'CBC' =>	'complete blood count',
			'CBG' =>	'capillary blood gas',
			'CC' =>		'chief complaint',
			'CCU' =>	'clean catch urine',
			'CCV' =>	'critical closing volume',
			'CF' =>		'cystic fibrosis',
			'CGL' =>	'chronic granulocytic leukemia',
			'CHF' =>	'congestive heart failure',
			'CHO' =>	'carbohydrate',
			'CI' =>		'cardiac index',
			'CML' =>	'chronic myelogenous leukemia',
			'CMV' =>	'cytomegalovirus',
			'CN' =>		'cranial nerves',
			'CNS ' =>	'central nervous system',
			'CO' =>		'cardiac output',
			'C/O' =>	'complaining of',
			'COLD' =>	'chronic obstructive lung disease',
			'COPD' =>	'chronic obstructive pulmonary disease',
			'CP' =>		'chest pain',
			'CPAP' =>	'continuous positive airway pressure',
			'CPK' =>	'creatinine phosphokinase',
			'CPR' =>	'cardiopulmonary resuscitation',
			'CRCL' =>	'creatinine clearance',
			'CRF' =>	'chronic renal failure',
			'CRP' =>	'C-reactive protein',
			'CSF' =>	'cerebrospinal fluid',
			'CT' =>		'computerized tomography',
			'CVA' =>	'cerebrovascular accident',
			'CVAT' =>	'CVA tenderness',
			'CVP' =>	'central venous pressure',
			'CXR' =>	'chest X-ray',
			'DAT' =>	'diet as tolerated',
			'DAW' =>	'dispense as written',
			'DC' =>		'discontinue',
			'D&C' =>	'dilation and curettage',
			'DDx' =>	'differential diagnosis',
			'D5W' =>	'5% dextrose in water',
			'DI' =>		'diabetes insipidus',
			'DIC' =>	'disseminated intravascular coagulopathy',
			'DIP' =>	'distal interphalangeal joint',
			'DJD' =>	'degenerative joint disease',
			'DKA' =>	'diabetic ketoacidosis',
			'dL' =>		'deciliter',
			'DM' =>		'diabetes mellitus',
			'DNR ' =>	'do not resuscitate',
			'DOA' =>	'dead on arrival',
			'DOE' =>	'dyspnea on exertion',
			'DPL' =>	'diagnostic peritoneal lavage',
			'DPT' =>	'diphtheria, pertussis, tetanus',
			'DTR' =>	'deep tendon reflexes',
			'DVT' =>	'deep venous thrombosis',
			'DX' =>		'diagnosis',
			'EAA' =>	'essential amino acids',
			'EBL' =>	'estimated blood loss',
			'ECG' =>	'electrocardiogram',
			'ECT' =>	'electroconvulsive therapy',
			'EFAD' =>	'essential fatty acid deficiency',
			'EMG' =>	'Electromyogram',
			'EMV' =>	'eyes, motor, verbal response (Glasgow coma scale)',
			'ENT' =>	'ears, nose, and throat',
			'EOM' =>	'extraocular muscles',
			'ESR' =>	'erythrocyte sedimentation rate',
			'ET' =>		'endotracheal',
			'ETT' =>	'endotracheal tube',
			'ERCP' =>	'endoscopic retrograde cholangio -pancreatography',
			'ETOH' =>	'ethanol',
			'EUA' =>	'examination under anesthesia',
			'FBS' =>	'fasting blood sugar',
			'FEV' =>	'forced expiratory volume',
			'FFP' =>	'fresh frozen plasma',
			'FRC' =>	'functional residual capacity',
			'FTT' =>	'failure to thrive',
			'FU' =>		'follow-up',
			'FUO' =>	'fever of unknown origin',
			'FVC' =>	'forced vital capacity',
			'Fx' =>		'fracture',
			'GC' =>		'gonorrhea',
			'GETT' =>	'general by endotracheal tube',
			'GFR' =>	'glomerular filtration rate',
			'GI' =>		'gastrointestinal',
			'gr' =>		'grain',
			'GSW' =>	'gun shot wound',
			'gt ' =>	'drops',
			'GTT' =>	'glucose tolerance test',
			'GU' =>		'genitourinary',
			'GXT' =>	'graded exercise tolerance (Stress test)',
			'HA' =>		'headache',
			'HAA' =>	'hepatitis  B surface antigen',
			'HAV' =>	'hepatitis A virus',
			'HBP' =>	'high blood pressure',
			'HCG' =>	'human chorionic gonadotropin',
			'HCT' =>	'hematocrit',
			'HDL' =>	'high density lipoprotein',
			'HEENT' =>	'head, eyes, ears, nose, throat',
			'Hgb' =>	'hemoglobin',
			'H/H' =>	'henderson-hasselbach equation',
			'HIV' =>	'human immunodeficiency virus',
			'HLA' =>	'histocompatibility locus antigen',
			'HJR' =>	'hepatojugular reflex',
			'HO' =>		'history of',
			'HOB' =>	'head of bed',
			'HPF' =>	'high power field',
			'HPI' =>	'history of present illness',
			'HR' =>		'heart rate',
			'HS' =>		'at bedtime',
			'HSM' =>	'hepatosplenomegaly',
			'HTLV-III' =>	'human lymphotropic virus, type III',
			'HSV' =>	'herpes simplex virus',
			'HTN' =>	'hypertension',
			'Hx' =>		'history',
			'I&D' =>	'incision and drainage',
			'I&O' =>	'intake and output',
			'ICS' =>	'intercostal space',
			'ICU' =>	'intensive care unit',
			'ID' =>		'infectious disease',
			'IDDM' =>	'insulin dependent diabetes mellitus',
			'IG' =>		'immunoglobulin',
			'IHSS' =>	'idiopathic hypertropic subaortic stenosis',
			'IM' =>		'intramuscular',
			'IMV' =>	'intermittent mandatory ventilation',
			'INF' =>	'intravenous nutritional fluid',
			'IPPB' =>	'intermittent positive pressure breathing',
			'IRBBB' =>	'incomplete right bundle branch block',
			'IRDM' =>	'insulin resistant diabetes mellitus',
			'ITP' =>	'idiopathic thrombocytopenic purpura',
			'IV' =>		'intravenous',
			'IVC' =>	'intravenous cholangiogram',
			'IVP' =>	'intravenous pyelogram',
			'tom' =>	'Tom Barrett',
		);

	}//end getAbbrevsDeprecated()


	// CORE THESAURUS RELATIONAL ENGINE

	/**
	* Get All Entities in this thesaurus
	*
	* @access public
	* @return array
	*/
	function getAllEntities()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = "SELECT id, name, description FROM !lex_thes_ent WHERE thes_id = ?";
		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,
		);

		$result = $db->getAll($sql, $data);

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

		$sql = "SELECT count(*) FROM !lex_thes_ent WHERE thes_id = ?";
		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,
		);

		$result = $db->getOne($sql, $data);

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
		if (empty($guess))
			return Array();

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = "SELECT id, name, description FROM !lex_thes_ent WHERE thes_id = ? AND name ILIKE '%!%'";
		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,

			$db->escapeSimple($guess),
		);

		$result = $db->getAll($sql, $data);

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

		$sql = 'SELECT id, name, description FROM !lex_thes_ent WHERE thes_id = ? AND id = ? ';

		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,

			$entity_id,
		);

		$result = $db->getRow($sql, $data);

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

		$sql = 'SELECT id, name, description FROM !lex_thes_ent WHERE thes_id = ? AND name = ? ';

		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,

			$name,
		);

		$result = $db->getRow($sql, $data);

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
				!lex_thes_ent_rel er,
				!lex_thes_ent e
			WHERE
					er.thes_id  = ?
				AND
					er.rel_id   = ?
				AND
					e.thes_id   = er.thes_id
				AND
					e.id         = er.cld_ent_id
		';

		$data = Array(
			SQ_TABLE_PREFIX,
			SQ_TABLE_PREFIX,
			$this->id,

			$relation_id,
		);

		// restrict the result to only the children of a given parent
		if (!empty($parent_id)) {
			$sql .= " and er.pnt_ent_id = ?";
			$data[] = $parent_id;
		}

		$result = $db->getAll($sql, $data);

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

		$sql = ' SELECT COUNT(*) FROM !lex_thes_ent_rel WHERE "thes_id"  = ? AND "rel_id"   = ? ';

		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,
			$relation_id,
		);

		// restrict the result to only the children of a given parent
		if (!empty($parent_id)) {
			$sql .= ' AND "pnt_ent_id" = ?';
			$data[] = $parent_id;
		}

		$result = $db->getOne($sql, $data);

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
				!lex_thes_ent_rel er,
				!lex_thes_ent e
			WHERE
					er.thes_id  = ?
				AND
					er.rel_id   = ?
				AND
					e.thes_id   = er.thes_id
				AND
					e.id        = er.pnt_ent_id

		';

		$data = Array(
			SQ_TABLE_PREFIX,
			SQ_TABLE_PREFIX,
			$this->id,

			$relation_id,
		);

		// restrict the result to only the parents of a given child
		if (!empty($child_id)) {
			$sql .= " and er.cld_ent_id = ?";
			$data[] = $child_id;
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
				!lex_thes_rel r,
				!lex_thes_ent_rel er
			WHERE
					r.thes_id     = ?
				AND
					er.cld_ent_id = ?
				AND
					er.thes_id    = r.thes_id
				AND
					er.reln_id    = r.id
		';

		$data = Array(
			SQ_TABLE_PREFIX,
			SQ_TABLE_PREFIX,
			$this->id,

			$child_entity_id,
		);

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

		$sql = "SELECT count(rel_id) FROM !lex_thes_ent_rel WHERE  thes_id = ? AND cld_ent_id = ?";

		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,

			$child_entity_id,
		);

		$result = $db->getOne($sql, $data);

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

		$sql = 'SELECT count(rel_id) FROM !lex_thes_ent_rel WHERE "thes_id" = ? AND "pnt_ent_id" = ?';

		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,

			$parent_entity_id,
		);

		$result = $db->getOne($sql, $data);

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

		$table_prefix = SQ_TABLE_PREFIX;

		$sql ='

			SELECT DISTINCT
				r.id,
				r.name
			FROM
				!lex_thes_rel r,
				!lex_thes_ent_rel er
			WHERE
					r.thes_id     = ?
				AND
					er.pnt_ent_id = ?
				AND
					er.thes_id    = r.thes_id
				AND
					er.rel_id     = r.id

		';

		$data = Array(
			SQ_TABLE_PREFIX,
			SQ_TABLE_PREFIX,
			$this->id,

			$parent_entity_id,
		);

		$result = $db->getAll($sql, $data);

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

		$sql = 'SELECT id, name FROM !lex_thes_rel where "thes_id" = ?';
		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,
		);

		$result = $db->getAll($sql, $data);

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

		$sql = "SELECT count(*) FROM !lex_thes_rel where thes_id = ?";
		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,
		);

		$result = $db->getOne($sql, $data);

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

		$sql = "SELECT id, name FROM !lex_thes_rel WHERE thes_id = ? AND id = ?";

		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,

			$id,
		);

		$result = $db->getRow($sql, $data);

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

			$sql = "SELECT id, name FROM !lex_thes_rel WHERE thes_id = ? AND name = ?";

			$data = Array(
				SQ_TABLE_PREFIX,
				$this->id,

				$name,
			);

			$result = $db->getRow($sql, $data);

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
				!lex_thes_ent e1,
				!lex_thes_ent e2,
				!lex_thes_ent_rel er
			WHERE
					er.thes_id = ?
				AND
					er.rel_id  = ?
				AND
					e1.id      = er.pnt_ent_id
				AND
					e2.id      = er.cld_ent_id
				AND
					e1.thes_id = er.thes_id
				AND
					e2.thes_id = er.thes_id

		';

		$data = Array(
			SQ_TABLE_PREFIX,
			SQ_TABLE_PREFIX,
			SQ_TABLE_PREFIX,
			$this->id,

			$relation_id,
		);

		$result = $db->getAll($sql, $data, DB_FETCHMODE_ORDERED);

		// check that result is not an error
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

		$id = $db->nextId(SQ_TABLE_PREFIX.'lex_thes_ent_id');
		assert_valid_db_result($id);

		$sql = 'INSERT INTO !lex_thes_ent ("thes_id", "id", "name", "description") VALUES (?,?,?,?)';

		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,

			$id,
			$name,
			$description,
		);

		$prepared = $db->prepare($sql);
		$result   = $db->execute($prepared,$data);

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

		$id = $db->nextId(SQ_TABLE_PREFIX.'lex_thes_rel_id');
		assert_valid_db_result($id);

		$sql = 'INSERT INTO !lex_thes_rel ("thes_id", "id", "name") VALUES (?,?,?)';

		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,

			$id,
			$name,
		);

		$prepared = $db->prepare($sql);
		$result   = $db->execute($prepared,$data);

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

			INSERT INTO !lex_thes_ent_rel
				("thes_id", "pnt_ent_id", "cld_ent_id", "rel_id")
			VALUES
				(?,?,?,?)

		';

		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,

			$parent_id,
			$child_id,
			$relation_id,
		);

		$prepared = $db->prepare($sql);
		$result   = $db->execute($prepared,$data);

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
				"id",
				"name",
				"description"
			FROM
				!lex_thes_ent
			WHERE
				"id" IN (
					SELECT DISTINCT
						"pnt_ent_id"
					FROM
						!lex_thes_ent_rel
					WHERE
						"thes_id" = ?
						AND "rel_id" = ?
						AND "pnt_ent_id" NOT IN (
							SELECT DISTINCT
								"cld_ent_id"
							FROM
								!lex_thes_ent_rel
							WHERE
								"thes_id" = ?
								AND "rel_id" = ?
						)
				)
		';

		$data = Array(
			SQ_TABLE_PREFIX,
			SQ_TABLE_PREFIX,
			$this->id,
			$relation_id,
			SQ_TABLE_PREFIX,
			$this->id,
			$relation_id,
		);

		$result = $db->getAll($sql, $data);
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
	* NOTICE: does not disable the thesaurus
	*
	* @access private
	* @return boolean
	*/
	function _deleteStoredContents()
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = '
			DELETE FROM !lex_thes_ent     WHERE "thes_id" = ?;
			DELETE FROM !lex_thes_rel     WHERE "thes_id" = ?;
			DELETE FROM !lex_thes_ent_rel WHERE "thes_id" = ?;
		';

		$data = Array(
			SQ_TABLE_PREFIX,
			$this->id,
			SQ_TABLE_PREFIX,
			$this->id,
			SQ_TABLE_PREFIX,
			$this->id,
		);

		$prepared = $db->prepare($sql);
		$result   = $db->execute($prepared,$data);

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

/*
This thesaurus has built-in support for Abbreviations and Categories
For the abbreviations to work, there needs to be an "Abbreviation"
relation

<thesaurus>
		<relations>
				<relation name="Child Relation" />
				<relation name="Parent" />
				<relation name="Friend" />
		</relations>
		<entities>
				<entity name="test1">
						<description>Description</description>
						<relation name="Child Relation">
								<entity name="test2" />
						</relation>
						<relation name="Parent">
								<entity name="test3" />
						</relation>
						<relation name="Friend">
								<entity name="test2" />
						</relation>
				</entity>

				<entity name="test2">
						<description>Description II</description>
						<relation name="Parent">
								<entity name="test1" />
						</relation>
						<relation name="Friend">
								<entity name="test1" />
								<entity name="test3" />
						</relation>
				</entity>

				<entity name="test3">
						<description>Description III</description>
						<relation name="Child Relation">
								<entity name="test1" />
						</relation>
						<relation name="Friend">
								<entity name="test2" />
						</relation>
				</entity>
				<entity name="Jora" />

		</entities>
</thesaurus>
*/

?>