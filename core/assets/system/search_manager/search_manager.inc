<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: search_manager.inc,v 1.2 2003/10/08 02:12:04 dwong Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Search_Manager
*
* Manages searching and relevance weights for attributes of asset types. 
*
*
* @author  Dominic Wong <dwong@squiz.net>
* @version $Revision: 1.2 $ - 0.1
* @package MySource_Matrix_Packages
* @subpackage __core__
*/

require_once SQ_INCLUDE_PATH."/asset.inc";
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
	} // end constructor


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


	/**
	* Prints out the backend interface for the search manager.
	* 
	* @see Asset::paintBackend()
	*
	* @param object Backend_Outputter $o
	*
	* @access public
	*/
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
	} // end paintBackend()

	/**
	* Processes a backend submission from this asset, returns true if all OK
	* Puts the array of processed attributes into $this->_tmp['process_attributes']
	*
	* @see Asset::processBackend()
	* @param object Backend_Outputter	$o
	* @param array(string)				&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/

	function processBackend(&$o, $link)
	{
		if (isset($_POST['asset_action']) && $_POST['asset_action'] == 'edit') {
			$edit_fns =& $this->getEditFns();
			$edit_fns->processBackendAssetWeights($this, $o);
		} else {
			parent::processBackend($o, $link);
		}
	} // end processBackend()

	/**
	* Function that massages database results.
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
	} // end massageResults()

}//end class
?>
