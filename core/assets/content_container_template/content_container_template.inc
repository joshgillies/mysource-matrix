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
*
*/

require_once SQ_INCLUDE_PATH.'/asset.inc';
/**
* Layout
*
* Purpose
*
*
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Content_Container_Template extends Asset
{


	/**
	* An array of keywords used in this layout
	* @var array
	*/
	public $layout_keywords = Array();


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{

		return Array(
				SQ_LINK_TYPE_1	=> Array(
									'asset'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
				SQ_LINK_TYPE_2	=> Array(
									'asset'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
				SQ_LINK_TYPE_3	=> Array(
									'asset'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
				SQ_LINK_NOTICE	=> Array(
									'asset'	=> Array(
												'card'		=> 'M',
												'exclusive'	=> FALSE,
											   ),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$attr_name = 'name';
		$values = $am->getAttributeValuesByName($attr_name, $this->type(), Array($this->id), 0);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name);
		} else {
			return $values[$this->id];
		}

	}//end _getName()

	/**
	* Returns root node links this template
	*
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	public function getRootNodeLinks()
	{
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'container_template_root');
		return $root_links;
	}//end getRootNodeLinks()


	/**
	* Returns metadata schemas from this template
	*
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	public function getMetadataSchemaLinks()
	{
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'container_template_metadata');
		return $root_links;
	}//end getMetadataSchemaLinks()


	/**
	* Returns simple edit layout links from this template
	*
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	public function getSimpleEditLayoutLinks()
	{
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'container_template_edit_layout');
		return $root_links;
	}//end getSimpleEditLayoutLinks()


	/**
	* Returns paint layout links from this template
	*
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	public function getPaintLayoutLinks()
	{
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'container_template_paint_layout');
		return $root_links;
	}//end getPaintLayoutLinks()


	/**
	* This function overides the default cloneLinks() in asset.inc
	* When you clone a template asset, you don't really want to clone the notice links which are used for marking "usage" purpose.
	* we don't want the cloned template to have same usage as original one
	*
	* @param Asset:OO	$clone	The asset object that we are turning into a clone of this one
	*
	* @return boolean
	* @access public
	*/
	public function cloneLinks(Asset $clone)
	{
		$orig_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', TRUE, 'major', NULL, FALSE, NULL);
		foreach ($orig_links as $orig_link) {
			// skip those notice links that is used as marking usage of the template
			if($orig_link['value'] === 'container_template') continue;

			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($orig_link['minorid'], $orig_link['minor_type_code']);
			if (is_object($asset) && $asset->canCloneLink()) {
				$linkid = $clone->createLink($asset, $orig_link['link_type'], $orig_link['value'], $orig_link['sort_order'], $orig_link['is_dependant'], $orig_link['is_exclusive']);
				if (!$linkid) return FALSE;
			}

		}//end foreach
		return TRUE;
	}

}//end class
?>
