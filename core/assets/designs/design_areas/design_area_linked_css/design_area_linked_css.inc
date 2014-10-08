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
* $Id: design_area_linked_css.inc,v 1.31 2013/01/16 06:12:54 akarelia Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Linked_CSS
*
* Purpose
* Allows you to link to a CSS file or customisation from a standard design
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.31 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Linked_Css extends Design_Area
{


	/**
	* Constructor
	*
	* @param string	$assetid	The asset ID to load
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Set the value for the passed variable to the passed value
	*
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	public function setAttrValue($name, $value)
	{
		// if they are trying to set the type_codes, split the string
		if ($name == 'type_codes') {
			if (!is_array($value)) {
				$value = preg_split('/[ ,]+/', (string) $value);
			}
			foreach ($value as $type_code) {
				if (!$GLOBALS['SQ_SYSTEM']->am->installed($type_code)) {
					trigger_localised_error('CORE0145', sprintf(translate('Asset Type "%s" is not installed in this system, unable to set "type_codes"'), $type_code), E_USER_WARNING);
					return FALSE;
				}
			}
		}

		return parent::setAttrValue($name, $value);

	}//end setAttrValue()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* In the form:
	*   Array('[link_type]' => Array('[type_code]' => '[cardinality]));
	* Where:
	*   link_type   = SQ_LINK_UNITE|SQ_LINK_USES|SQ_LINK_NOTICE
	*   cardinality = 1|M
	*
	* @return array
	* @access private
	*/
	public function _getAllowedLinks()
	{

		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array(
									'asset'	=> Array(
												'card'		=> 1,
												'exclusive'	=> FALSE,
											   ),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Returns the particular info for the linked CSS
	*
	* Returns NULL if no link found
	*
	* @param string	$field	field in the link that is required (any valid field from getLink())
	*
	* @return mixed NULL|string
	* @access public
	* @see Asset::getLink()
	*/
	public function getLinkedCSSInfo($field)
	{
		if (empty($this->_tmp['css_assetid']) || (!isset($this->_tmp['css_assetid']))) {
			$this->_tmp['css_assetid'] = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, 'linked_css_asset');
		}
		return (empty($this->_tmp['css_assetid'])) ? NULL : $this->_tmp['css_assetid'][$field];

	}//end getLinkedCSSInfo()


	/**
	* Called whenever any type of link is changed
	*
	* @return void
	* @access private
	*/
	public function linksUpdated()
	{
		parent::linksUpdated();
		unset($this->_tmp['css_assetid']);

	}//end linksUpdated()


	/**
	* Print code that actually results in HTML output to the browser for this design area
	* This function is called at the AREA tag if print != no, and at the point of any PRINT
	* tags that reference this area.
	*
	* This function can be overridden (partially) by children
	*
	* @return void
	* @access protected
	*/
	protected function _printPaintingCode()
	{
		$css_assetid = (int) $this->getLinkedCSSInfo('minorid');

		// return true because it just means that they haven't set anything yet
		if (empty($css_assetid)) return TRUE;

		$css_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($css_assetid);
		if(empty($css_asset) || !method_exists($css_asset, 'getCacheFileId')) {
			trigger_localised_error('CORE0329', sprintf(translate('Linked Design CSS #%s does not exist in the system.'), $css_assetid), E_USER_NOTICE);
			return;
		}
		$cache_file_assetid = $css_asset->getCacheFileId();

		// The code we are about to add to the design file will check if caching
		// is enabled for the css asset and if so will try to get a URL
		// from the cache file asset.  But if for any reason it didn't get a
		// URL for the cache file asset it will use the URL of the CSS asset itself.

		echo '<', '?php
		$linked_css_'.$this->attr('id_name').'_url = $GLOBALS[\'SQ_SYSTEM\']->am->getAssetURL('.$cache_file_assetid.');
		$css_attr = $GLOBALS[\'SQ_SYSTEM\']->am->getAllAttributeValues('.$css_assetid.', \''.$css_asset->type().'\');

		if (!empty($linked_css_'.$this->attr('id_name').'_url)) {
			$served_by_apache = strpos($linked_css_'.$this->attr('id_name').'_url, \'/__data/\') !== FALSE || (SQ_CONF_STATIC_ROOT_URL && strpos($linked_css_'.$this->attr('id_name').'_url, SQ_CONF_STATIC_ROOT_URL.\'/\') !== FALSE);
			if ($served_by_apache && (isset($css_attr[\'static_url_versioning\'][\'value\']) &&  ($css_attr[\'static_url_versioning\'][\'value\'] == 1))) {
				$cache_file_version = $GLOBALS[\'SQ_SYSTEM\']->am->getAssetInfo('.$cache_file_assetid.', Array(), TRUE, \'version\');
				if (!empty($cache_file_version['.$cache_file_assetid.'])) $linked_css_'.$this->attr('id_name').'_url .= \'?v=\'.$cache_file_version['.$cache_file_assetid.'];
			}
		}

		if (empty($linked_css_'.$this->attr('id_name').'_url) || (isset($css_attr[\'cache_css\'][\'value\']) &&  ($css_attr[\'cache_css\'][\'value\'] == 0))) {
			$linked_css_'.$this->attr('id_name').'_url = $GLOBALS[\'SQ_SYSTEM\']->am->getAssetURL('.$css_assetid.');
		}

		if (!empty($linked_css_'.$this->attr('id_name').'_url)) {
			','?', '>
			';

			switch ($this->attr('tag_type')) {
				case 'link_tag' :
					echo '<link rel="stylesheet" type="text/css" href="', '<', '?php echo $linked_css_'.$this->attr('id_name').'_url; ?', '>', '"';
					echo ' media="', $this->attr('media'), '" />
					';
				break;
				case 'import_tag' :
					echo '@import url(', '<', '?php echo $linked_css_'.$this->attr('id_name').'_url; ?', '>);
					';
				break;
			} //end switch

		echo '<', '?php
		}
		?', '>';

	}//end _printPaintingCode()


}//end class
?>
