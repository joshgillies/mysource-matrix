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
* $Id: content_type.inc,v 1.32 2012/08/30 01:09:05 ewang Exp $
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
* @version $Revision: 1.32 $
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
			trigger_localised_error('CORE0083', sprintf(translate('Unable to create a %s with a blank name'), $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name')), E_USER_WARNING)
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
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

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
	* @param boolean $empty_content If TRUE, empty content will be returned
	*
	* @return string
	* @access public
	*/
	public function getContent($empty_content=TRUE)
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
				$contextid = (int)$GLOBALS['SQ_SYSTEM']->getContextId();
				$datapath = $container->data_path;

				// if default context attribute value changed
				// remove all content files for alternate contexts to stay in sync with new attributes #3967
				if($contextid === 0 && !empty($datapath)) {
					$contextids = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());
					foreach ($contextids as $id) {
						$file = $datapath.'/content_file.'.$id.'.php';
						if(file_exists($file)) 	unlink($file);
					}

				}
				// generate content file now
				$edit_fns = $container->getEditFns();
				$edit_fns->generateContentFile($container);
			}
		}

		$success = parent::saveAttributes($dont_run_updated);

		if ($contents_changed && $success) {
			$em = $GLOBALS['SQ_SYSTEM']->getEventManager();
			$em->broadcastEvent($this, 'ContentsUpdated');

			// broadcast conetnt chnaged trigger event for its top level parent
			$top_level_parents = $GLOBALS['SQ_SYSTEM']->am-> getDependantParents($this->id, '', TRUE, FALSE);
			foreach ($top_level_parents as $parentid) {
				$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parentid);
				$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_asset_content_changed', $parent);
			}
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

		foreach ($content_types as $index => $content_type) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($index);
			$new_content_type = new $index();
			if ($new_content_type->deprecated === TRUE) {
				unset($content_types[$index]);
			}
		}
		return $content_types;

	}//end getAvailableContentTypes()

	/**
    * This function only takes care of 'html' attribute by checking for hyperlinks pointing to non-live assets and
    * by passing to the parent function. Other attributes will be passed directly to the parent function.
    *
	* @param string	$name	the name of the attribute
	* @param string	$value	the new value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value)
	{
		if($this instanceof Content_Type_Nest_Content) return parent::setAttrValue($name, $value);

		$live_link_only = $GLOBALS['SQ_SYSTEM']->getUserPrefs('content_type_wysiwyg', 'SQ_LIVE_LINK_ONLY');
		if ($live_link_only && $this->status == SQ_STATUS_LIVE && $name === 'html') {
			require_once SQ_FUDGE_PATH.'/general/www.inc';
			$outcome = check_non_live_link($value, $this->id);
			if ($outcome !== FALSE) $value = $outcome;
		}
		return parent::setAttrValue($name, $value);

	}//end setAttrValue()

}//end class

?>
