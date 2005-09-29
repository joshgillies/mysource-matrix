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
* $Id: data_source_record_set.inc,v 1.2 2005/09/29 00:29:20 pgannavarapu Exp $
*
*/



/**
* Data_Source_Record_Set
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.2 $
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
		$this->_ser_attrs = true;
		$this->_loadVars();

		if (!$data_sourceid || empty($data)) {
			return;
		}

		$data_source = &$GLOBALS['SQ_SYSTEM']->am->getAsset($data_sourceid, 'data_source');
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
		//if the name has keywords in it then we get the string with replacements
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
			$this->name = 'Record Set '.$this->id;
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
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	function _getName($short_name=false)
	{
		return $this->name;

	}//end _getName()


	/**
	* Return the keywords for this asset type.
	*
	* This function is called in asset-listing situations like the asset listing, search page etc
	*
	* The return value is in the form:
	* <pre>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </pre>
	*
	* @access public
	* @return array
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
		//return parent::getAvailableKeywords();
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
		$replacement = null;
		if (strpos($keyword, 'data_source_record_set_') !== false) {
			$keyword = substr($keyword, strlen('data_source_record_set_'));
			$replacement = $this->vars[$keyword];
		} else if (strpos($keyword, 'asset_') !== false) {
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


}//end class

?>
