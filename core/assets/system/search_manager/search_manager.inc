<?php

/**
* Search_Manager
*
* Purpose
*
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Version$ - 0.1
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Search_Manager extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Search_Manager($assetid=0)
	{
		$this->Asset($assetid);
	}


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

		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$where = 'type_code = '.$db->quote($this->type());
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);
		$sql = 'SELECT COUNT(*) FROM '.SQ_TABLE_RUNNING_PREFIX.'asset '.$where;
		$exists = $db->getOne($sql);
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br/>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			trigger_error('Unable to create another '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').', you are only allowed one', E_USER_WARNING);
			return false;
		}

		$ret_val = parent::create($link);

		return $ret_val;

	}//end create()


	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name');
	}//end _getName();


	function paintBackend(&$o) 
	{
		$asset_ei_screen = '';
		if (isset($_REQUEST['asset_ei_screen'])) {
			$asset_ei_screen = $_REQUEST['asset_ei_screen'];
		}
		switch ($asset_ei_screen) {
			case 'weights':
				$current_location = $o->getCurrentLocation();
				$current_location .= (strstr($current_location, '?')) ? '&' : '?';
				//$o->setCurrentScreen($current_location.'asset_ei_screen='.rawurlencode('weights'));
				$o->addFormActionGetVar('asset_ei_screen', 'weights');
				$edit_fns =& $this->getEditFns();
				if (!$edit_fns) {
					trigger_error("Edit function not found for Search Manager.", E_USER_ERROR);
					break;
				}
				$o->addHiddenField('asset_action', (($this->id) ? 'edit' : 'create'));
				$edit_fns->paintBackendAssetWeights($this, $o);
				break;

			default:
				parent::paintBackend($o);
				break;
		}
	}

	function processBackend(&$o, $link)
	{
		if (isset($_POST['asset_action']) && $_POST['asset_action'] == 'edit') {
			$edit_fns =& $this->getEditFns();
			$edit_fns->processBackendAssetWeights($this, $o);
		} else {
			parent::processBackend($o, $link);
		}
	}

	/**
	* Function that massages results.
	* 
	* @param array $results The rows returned from the database.
	* @returns array Massaged results in array of assetid => components
	* @access private
	*/
	function &massageResults(&$results) 
	{
		if (DB::isError($results)) {
			trigger_error('DB error while searching.', E_USER_WARNING);
			return array();
		}

		# need to massage the data a little
		$matches = array();
		while ($row = $results->fetchRow()) {
			$assetid	= $row['assetid'];
			$component	= $row['component'];
			$type_code  = $row['type_code'];
			$score		= $row['score'];

			if (!isset($matches[$assetid]))
				$matches[$assetid] = array();
			if (!in_array($component, $matches[$assetid]))
				$matches[$assetid]['component'] = $component;
				$matches[$assetid]['type_code'] = $type_code;
				$matches[$assetid]['score']		= $score;
		}

		$results->free();

		return $matches;
	}

	/**
	* Executes a keydata search through the index.
	* 
	* @access public
	* @param array $words
	* @param array $exclude_words
	* @returns array An array of assetid => components
	*/
	function keyWordsMatch($words, $exclude_words = Array(), $component_words = Array(), $whole_words = true, $assetTypes = false, $restrictAssetIds = null, $components = false)
	{
		if (!is_array($words))
			return null;
		if (empty($words))
			return array();

		$base_sql = "SELECT ai.assetid, ai.component, a.type_code
				FROM " .
				SQ_TABLE_PREFIX ."asset_indexing ai, " .
				SQ_TABLE_PREFIX ."asset a
				WHERE ai.assetid = a.assetid
				AND ai.type = 'text' \n";

		$end_sql = '';

		if ($assetTypes !== false && is_array($assetTypes)) {
			$end_sql .= "AND a.type_code IN ('" . implode("', '", $assetTypes) . "')\n";
		}
		if (!is_null($restrictAssetIds) && is_array($restrictAssetIds)) {
			$end_sql .= "AND ai.assetid IN ('". implode("', '", $restrictAssetIds). "')\n";
		}

		$include_sql = '';
		$include_word_list = array();
		foreach ($words as $word) {
			if ($include_sql != '')
				$include_sql .= " INTERSECT ";

			array_push($include_word_list, $word);

			$include_sql .= "(" .$base_sql .
				"AND ai.value LIKE '%" .$word ."%'\n". $end_sql. ")\n";
		}


		$exclude_sql = '';
		foreach ($exclude_words as $word) {
			if ($exclude_sql != '')
				$exclude_sql .= " UNION ";
			
			$exclude_sql .= "(" .$base_sql .
				"AND ai.value LIKE '%" .$word ."%'\n" .$end_sql. ")\n";

		}


		$search_subquery = $include_sql;
		if ($exclude_sql != '')
			$search_subquery .= " EXCEPT " .$exclude_sql;

		$sql = "SELECT results.*, sum(ai.score) as score
				FROM ($search_subquery) results, " .SQ_TABLE_PREFIX ."asset_indexing ai
				WHERE ai.assetid = results.assetid AND ai.component = results.component AND ai.type = 'text'
				AND (";

		$first = true;
		foreach ($include_word_list as $word) {
			if (!$first)
				$sql .= " OR ";
			$first = false;
			$sql .= "ai.value LIKE '%". $word ."%'";
		}

		$sql .= ") $end_sql
				GROUP BY results.assetid, results.component, results.type_code
				ORDER BY score DESC";

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		bom($sql);
		$results = $db->query($sql);
		$results = $this->massageResults($results);

		// check permissions
		foreach ($results as $assetid => $result) {
			$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
			bom($asset->name . " / " .$asset->id);
			if (!$asset->readAccess()) {
				unset($results[$assetid]);
			}
		}
		
		return $results;
	}

	function keyDateRangeMatch($startDate, $endDate, $assetTypes = false, $components = false)
	{

		$sql = "SELECT ai.assetid, ai.component 
				FROM ".
				SQ_TABLE_PREFIX."asset_indexing ai, ". 
				SQ_TABLE_PREFIX . "asset a
				WHERE ai.assetid = a.assetid
				AND ai.type = 'date'
				AND TO_DATE(ai.value, 'YYYY-MM-DD HH:II:SS') >= TO_DATE('" . addslashes($startDate)	. "', 'YYYY-MM-DD HH:II:SS')
				AND TO_DATE(ai.value, 'YYYY-MM-DD HH:II:SS') <  TO_DATE('" . addslashes($endDate)	. "', 'YYYY-MM-DD HH:II:SS')\n";

		if ($assetTypes !== false && is_array($assetTypes)) {
			$sql .= "AND a.type_code IN ('" . implode("', '", $assetTypes) . "')\n";
		}

		if ($components !== false && is_array($components)) {
			$sql .= "AND ai.component IN ('" . implode("', '", $components) . "')\n";
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$results = $db->query($sql);

		return $this->massageResults($results);
		
	}

	function keyNumberRangeMatch($lo, $hi, $lo_inclusive = false, $hi_includsive = false, $assetTypes = false, $components = false)
	{
		$lo_op = ">";
		$hi_op = "<";

		if ($lo_inclusive) {
			$lo_op .= "=";
		}
		if ($hi_inclusive) {
			$hi_op .= "=";
		}

		$sql = "SELECT ai.assetid, ai.component 
				FROM ".
				SQ_TABLE_PREFIX."asset_indexing ai, ". 
				SQ_TABLE_PREFIX . "asset a
				WHERE ai.assetid = a.assetid
				AND ai.type = 'number'
				AND ai.value $lo_op '" . addslashes($lo)	. "'
				AND ai.value $hi_op '" . addslashes($hi)	. "' \n";

		if ($assetTypes !== false && is_array($assetTypes)) {
			$sql .= "AND a.type_code IN ('" . implode("', '", $assetTypes) . "')\n";
		}

		if ($components !== false && is_array($components)) {
			$sql .= "AND ai.component IN ('" . implode("', '", $components) . "')\n";
		}

		$db = &$GLOBALS['SQ_SYSTEM']->db;
		$results = $db->query($sql);

		return $this->massageResults($results);
		
	}


}//end class
?>
