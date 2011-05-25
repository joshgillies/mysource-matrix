<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: content_type_nest_content.inc,v 1.26.12.1 2011/05/25 23:27:39 akarelia Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type.inc';

/**
* Content_Type_Nest_Content
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.26.12.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Nest_Content extends Content_Type
{


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
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return void
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_NOTICE	=> Array(
									'asset'	=> Array(
												'card'		=> 1,
												'exclusive'	=> FALSE,
											   ),
								   ),
			   );

	}//end _getAllowedLinks()


	/**
	* Returns true if there is no nested assetid associated with this content type
	*
	* @return boolean
	* @access public
	*/
	function isEmpty()
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, 'nested_asset');
		return empty($link);

	}//end isEmpty()


	/**
	* Return a human readable description of the passed link
	*
	* @param int	$linkid	the link ID of the link to describe
	*
	* @return string
	* @access public
	*/
	function describeLink($linkid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		if ($link['value'] == 'nested_asset') {
			return translate('cms_ct_nest_content_link_desc');
		}

	}//end describeLink()


	/**
	* Returns the editable content of this asset
	*
	* @return string
	* @access public
	*/
	function getContent()
	{
		return '';

	}//end getContent()


	/**
	* Sets the editable content of this asset
	*
	* @return boolean
	* @access public
	*/
	function setContent()
	{
		return FALSE;

	}//end setContent()


	/**
	* Allow any required processesing to occur when any link is updated for the asset
	* Called whenever any type of link is changed
	*
	* @return void
	* @access protected
	*/
	public function linksUpdated()
	{
		// this function is called, which mean the asset we are
		// nesting is modified (deleted) update our content file
		$bodycopy_div_id = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'bodycopy_div', TRUE));
		if (!empty($bodycopy_div_id)) {
			$bodycopy_div = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_div_id[0]);
			$bodycopy_div_edit_fns = $bodycopy_div->getEditFns();
			$all_context = $GLOBALS['SQ_SYSTEM']->getAllContexts();
			foreach ($all_context as $context_id => $info) {
				$GLOBALS['SQ_SYSTEM']->changeContext($context_id);
					$bodycopy_div_edit_fns->generateContentFile($bodycopy_div);
				$GLOBALS['SQ_SYSTEM']->restoreContext();
			}
		}

		return parent::linksUpdated();

	}//end linksUpdated()


}//end class

?>
