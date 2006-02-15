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
* $Id: data_source_record_set.inc,v 1.7 2006/02/15 03:03:40 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';


/**
* Data_Source_Record_Set
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage core
*/
class Data_Source_Record_Set extends Asset
{

	/**
	* The ID of the bridge we are from
	*
	* @var int
	*/
	var $_data_sourceid = 0;


	/**
	* Constructor
	*
	* @param int	$data_source_id	the ID of the Data Source bridge that created this record asset
	* @param array	$data	the record details for this record. contains all info like the field names and the values
	*
	*/
	function Data_Source_Record_Set($data_sourceid=0, $data=Array())
	{
		$this->_ser_attrs = TRUE;
		$this->_loadVars();

		if (!$data_sourceid || empty($data)) {
			return;
		}

		$data_source =& $GLOBALS['SQ_SYSTEM']->am->getAsset($data_sourceid);
		if (is_null($data_source)) return;

		$this->_data_sourceid = $data_sourceid;

		// set general object properties
		$this->id = $data_source->id.':'.$data['shadow_asset_id'];
		unset($data['shadow_asset_id']);


		$this->status = $data_source->status;
		$this->version = '0.1';

		foreach ($data as $key => $value) {
			$this->vars[$key] = $value;
		}
		// if the name has keywords in it then we get the string with replacements
		if (!empty($data['shadow_asset_name'])) {
			$name = $data['shadow_asset_name'];
			require_once SQ_FUDGE_PATH.'/general/text.inc';
			$keywords = retrieve_keywords_replacements($name);
			$replacements = Array();
			foreach ($keywords as $key) {
				$replacements[$key] = $this->getKeywordReplacement($key);
			}
			$name_string = replace_keywords($name, $replacements);
			$this->name = $name_string;
		} else {
			if (isset($data['title'])) {
				$this->name = $data['title'];
			} else {
				$this->name = 'Data Source Record Set '.$this->id;
			}
		}
		$this->short_name = $this->name;

	}//end constructor


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array();

	}//end _getAllowedLinks()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=FALSE)
	{
		return $this->name;

	}//end _getName()


	/**
	* Return the keywords for this asset type.
	* This function is called in asset-listing situations like the asset listing, search page etc
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </pre>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$keywords = Array();
		if (empty($this->vars)) {
			$keywords = parent::getAvailableKeywords();
			return $keywords;
		}
		$columns = array_keys($this->vars);
		$keywords['asset_assetid'] = translate('record_set_id');
		$keywords['asset_version'] = translate('record_set_version');
		$keywords['asset_status'] = translate('record_set_status');
		$keywords['asset_name'] = translate('record_set_name');
		$keywords['asset_short_name'] = translate('record_set_short_name');
		$keywords['asset_data_source_id'] = translate('data_source_id');
		foreach ($columns as $column) {
			$keywords['data_source_record_set_'.$column] = translate('record_set_attribute').$column;
		}
		return $keywords;

	}//end getAvailableKeywords()


	/**
	* Get bodycopy keyword replacement
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		$replacement = NULL;
		if (strpos($keyword, 'data_source_record_set_') !== FALSE) {
			$keyword = substr($keyword, strlen('data_source_record_set_'));
			if (isset($this->vars[$keyword])) {
				$replacement = $this->vars[$keyword];
			} else {
				$replacement = '';
			}
		} else if (strpos($keyword, 'asset_') !== FALSE) {
			$key = substr($keyword, strlen('asset_'));
			switch ($key) {
				case 'name':
				case 'short_name':
					$replacement = $this->name;
				break;
				case 'assetid':	$replacement = $this->id;
				break;
				case 'version':	$replacement = $this->version;
				break;
				case 'status':	$replacement = $this->status;
				break;
				case 'data_source_id':
					$replacement = $this->_data_sourceid;
				break;
				default: $replacement = parent::getKeywordReplacement($keyword);
			}
		}
		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Returns TRUE if at least one of the passed assetids has read access to this asset
	*
	* @param array	$assetids	An array of user_group and/or user ids to check for read access
	*							If the array is empty, the current user will be checked
	*
	* @return boolean
	* @access public
	*/
	function readAccess($assetids=Array())
	{
		$parent =& $this->_getBridge();
		if (!is_null($parent)) {
			$result = $parent->readAccess($assetids);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
			return $result;
		}

		return FALSE;

	}//end readAccess()


	/**
	* Returns TRUE if at least one of the passed assetids has write access to this asset
	*
	* The extra parameter, $only_workflow, is currently only be used in
	* determining read access for an asset before it goes live.
	*
	* @param string		$lock_type		the type of write access required - if blank returns a plain check against
	*									permissions set for this asset, otherwise returns effective access based
	*									on locking and status
	* @param array		$assetids		An array of user_group and/or user ids to check for write access
	*									If the array is empty, the current user will be checked
	* @param boolean	$only_workflow	If TRUE (default), users with write access normally will be denied write access
	*									if outside the current step of the workflow. If FALSE, both those in the workflow
	*									and those with normal write access will be granted write access.
	*
	* @return boolean
	* @access public
	*/
	function writeAccess($lock_type='', $assetids=Array(), $only_workflow=TRUE)
	{
		$parent =& $this->_getBridge();
		if (!is_null($parent)) {
			$result = $parent->writeAccess($lock_type, $assetids, $only_workflow);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent);
			return $result;
		}

		return FALSE;

	}//end writeAccess()


	/**
	* returns the owning bridge asset of this shadow asset
	*
	* @return mixed object|NULL
	* @access private
	*/
	function &_getBridge()
	{
		list($bridgeid, $childid) = explode(':', $this->id);
		$asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($bridgeid);
		return $asset;

	}//end _getBridge()


}//end class

?>
