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
* $Id: design_area_nest_content.inc,v 1.31 2012/08/30 01:09:06 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Nest_Content
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.31 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Nest_Content extends Design_Area
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
					trigger_localised_error('CORE0145', sprintf(translate('Asset Type "%s" is not installed in this system, unable to set "type_codes"'), $type_code), E_USER_WARNING)
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
	* Returns the particular info for the nested assets
	*
	* Returns NULL if no link found
	*
	* @param boolean	$field	field in the link that is required (any valid field from getLink())
	*
	* @return mixed NULL|string|object
	* @access private
	* @see Asset::getLink()
	*/
	public function _getNestAssetInfo($field)
	{
		if ((empty($this->_tmp['nest_assetid'])) || (!isset($this->_tmp['nest_assetid']))) {
			$this->_tmp['nest_assetid'] = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, 'nest_content_asset');
		}
		return (empty($this->_tmp['nest_assetid'])) ? NULL : $this->_tmp['nest_assetid'][$field];

	}//end _getNestAssetInfo()


	/**
	* Called whenever any type of link is changed
	*
	* @return void
	* @access private
	*/
	public function linksUpdated()
	{
		parent::linksUpdated();
		unset($this->_tmp['nest_assetid']);

	}//end linksUpdated()


	/**
	* Return whether this DA has been given sufficient details to be able to print stuff
	*
	* @return boolean
	* @access protected
	*/
	protected function _canInit()
	{
		$assetid = (int) $this->_getNestAssetInfo('minorid');
		return !(empty($assetid));

	}//end _canInit()


	/**
	* Print code that initialises this design area and should always be printed in place of the AREA tag
	*
	* @return void
	* @access protected
	*/
	protected function _printInitCode()
	{
		$assetid = (int) $this->_getNestAssetInfo('minorid');
		$paint_layout_id = (int) $this->attr('paint_layout');
		$paint_with = (string) $this->attr('paint_with');
		echo '<', '?php
		$nest_content_'.$this->attr('id_name').'_paint_layout = NULL;
		$nest_content_'.$this->attr('id_name').'_asset = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset('.$assetid.');';
		if (!empty($paint_layout_id) && $paint_with == 'arbitrary') {
			echo '$nest_content_'.$this->attr('id_name').'_paint_layout = $GLOBALS[\'SQ_SYSTEM\']->am->getAsset('.$paint_layout_id.');';
		}
		echo '$nest_content_'.$this->attr('id_name').'_paint_with = \''.$paint_with.'\';
		?', '>';

	}//end _printInitCode()


	/**
	* Print code that actually results in HTML output to the browser for this design area
	*
	* This function is called at the AREA tag if print != no, and at the point of any PRINT
	* tags that reference this area.
	*
	* @return void
	* @access protected
	*/
	protected function _printPaintingCode()
	{
		echo '<', '?php
		if (!is_null($nest_content_'.$this->attr('id_name').'_asset)) {
			if ($nest_content_'.$this->attr('id_name').'_asset->readAccess()) {
				ob_start();
					switch ($nest_content_'.$this->attr('id_name').'_paint_with) {
						case \'arbitrary\':
							if (!is_null($nest_content_'.$this->attr('id_name').'_paint_layout)) {
								$switched_frontend = FALSE;
								if ($GLOBALS[\'SQ_SYSTEM\']->frontend_asset->id !== $this->id) {
									$current_frontend_asset =& $GLOBALS[\'SQ_SYSTEM\']->frontend_asset;
									$GLOBALS[\'SQ_SYSTEM\']->frontend_asset =& $nest_content_'.$this->attr('id_name').'_asset;
									$switched_frontend = TRUE;
								}
								$nest_content_'.$this->attr('id_name').'_paint_layout->paint($nest_content_'.$this->attr('id_name').'_asset);

								if ($switched_frontend) {
									$GLOBALS[\'SQ_SYSTEM\']->frontend_asset =& $current_frontend_asset;
								}
							} else {
								$nest_content_'.$this->attr('id_name').'_asset->printBody();
							}
						break;

						case \'default\':
							$nest_content_'.$this->attr('id_name').'_asset->printBodyWithPaintLayout();
						break;

						default:
							$nest_content_'.$this->attr('id_name').'_asset->printBody();
						break;
					}
				$body = ob_get_clean();
				$GLOBALS[\'SQ_SYSTEM\']->replaceKeyword($body);

				echo $body;
			}
		}
		?', '>';

	}//end _printPaintingCode()


}//end class
?>
