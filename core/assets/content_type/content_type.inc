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
* $Id: content_type.inc,v 1.22 2008/09/22 03:28:36 ata Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';

/**
* Content_Type
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.22 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Content_Type extends Asset
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Content types ensure a name has been set before they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	protected function _preCreateCheck(Array &$link)
	{
		if (!parent::_preCreateCheck($link)) return FALSE;

		$name = trim($this->attr('name'));
		if ($name == '') {
			trigger_localised_error('CORE0083', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array();

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
	protected function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Returns TRUE if there is no content
	*
	* @return boolean
	* @access public
	*/
	public function isEmpty()
	{
		$html = trim($this->attr('html'));
		return empty($html);

	}//end isEmpty()


	/**
	* Returns the editable content of this asset
	*
	* @return string
	* @access public
	*/
	public function getContent()
	{
		return trim($this->attr('html'));

	}//end getContent()


	/**
	* Sets the editable content of this asset
	*
	* @param string	$content	the content to set
	*
	* @return boolean
	* @access public
	*/
	public function setContent($content)
	{
		$this->setAttrValue('html', $content);
		$this->saveAttributes();

		return TRUE;

	}//end setContent()


	/**
	* Save attributes set with setAttrValue to the database
	*
	* @param boolean	$dont_run_updated	if TRUE this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see setAttrValue()
	*/
	public function saveAttributes($dont_run_updated=FALSE)
	{
		$contents_changed = isset($this->_tmp['vars_set']['html']);

		// Want to see if the content has been changed.
		if ($contents_changed) {
			// Yep, so we need to (re)generate our bodycopy container's content file
			$bc_container_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE, 'minor');
			if (!empty($bc_container_link)) {
				$container = $GLOBALS['SQ_SYSTEM']->am->getAsset($bc_container_link[0]['majorid']);
				$edit_fns = $container->getEditFns();
				$edit_fns->generateContentFile($container);
			}
		}

		$success = parent::saveAttributes($dont_run_updated);

		if ($contents_changed && $success) {
			$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($this, 'ContentsUpdated');
		}

		return $success;

	}//end saveAttributes()


	/**
	* This function mainly exists to update the paint layout bodycopy
	*
	* @return void
	* @access public
	*/
	public function linksUpdated()
	{
		// Bug Fix 1541
		// Paintlayout bodycopy is not updated if the content type is updated but its attributes
		// was not changed. For instance, the nested content type, it does not change any of its attributes
		// although you change the asset to nest within. Consequently, saveAttributes() function can not
		// propagate to all the parents resulting in paint layout bodycopy unchanged.
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$bc_container_link = $am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE, NULL, 'minor');

		if (!empty($bc_container_link)) {
			$bc_container = $am->getAsset($bc_container_link['majorid']);
			$bc_link = $am->getLink($bc_container->id, SQ_LINK_TYPE_2, 'paint_layout_bodycopy', TRUE, NULL, 'minor');

			if (!empty($bc_link)) {
				$bodycopy = $am->getAsset($bc_link['majorid']);
				$bodycopy->_updated();
			}
		}
		return TRUE;

	}//end linksUpdated()

	
	/**
	 * Get all the available content types in the system
	 *
	 * @return array	all the content types available in the system
	 * @access public
	 */
	public static function getAvailableContentTypes() {
		$content_types = $GLOBALS['SQ_SYSTEM']->am->getAssetTypeHierarchy('content_type');
		
		if (isset($content_types['content_type_markdown'])) {
			//if markdown is not enabled, remove it from the content type
			$disable_markdown = TRUE;
			require_once SQ_DATA_PATH.'/private/conf/tools.inc';
			if (SQ_TOOL_MARKDOWN_ENABLED && is_file(SQ_TOOL_MARKDOWN_PATH)) {
				include_once SQ_TOOL_MARKDOWN_PATH;
				if (function_exists('Markdown')) {
					$disable_markdown = FALSE;
				}
			}
			if ($disable_markdown) {
				unset($content_types['content_type_markdown']);
			}
		}
		
		return $content_types;
		
	}//end getAvailableContentTypes()
	

}//end class

?>
