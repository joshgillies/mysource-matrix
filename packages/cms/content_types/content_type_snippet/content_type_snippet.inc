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
* $Id: content_type_snippet.inc,v 1.12 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type.inc';

/**
* Content_Type_Raw_HTML
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Snippet extends Content_Type
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
	* Allow any required processesing to occur when any link is updated for the asset
	*
	* @return void
	* @access protected
	*/
	function linksUpdated()
	{
		// update content.php file once snippet notice link has been deleted
		$bc_container_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE, 'minor');
		if (!empty($bc_container_link)) {
			$container = $GLOBALS['SQ_SYSTEM']->am->getAsset($bc_container_link[0]['majorid']);
			$edit_fns = $container->getEditFns();
			$edit_fns->generateContentFile($container);
		}

	}//end linksUpdated()


	/**
	* Returns true if there is no snippet selected
	*
	* @return boolean
	* @access public
	*/
	function isEmpty()
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, 'snippet_asset');
		return empty($link);

	}//end isEmpty()


	/**
	* Returns an array of snippet name and content, indexed by id
	*
	* @return array
	* @access public
	*/
	function getSnippets()
	{
		$rootids = explode(',', $GLOBALS['SQ_SYSTEM']->getUserPrefs($this->type(), 'SQ_SNIPPET_ROOT'));
		if (empty($rootids)) {
			// snippet root preference not set
			return Array();
		}
		// Validate the root node assetids
		$rootids = array_keys($GLOBALS['SQ_SYSTEM']->am->getAssetInfo($rootids));

		$containers = Array();
		foreach($rootids as $rootid) {
			$containers += $GLOBALS['SQ_SYSTEM']->am->getChildren($rootid, 'bodycopy_container', FALSE);
		}
		if (empty($containers)) {
			// no container found under the specified snippet root
			return Array();
		}

		// generate list of snippet name and content
		$results = Array();
		foreach ($containers as $id => $type_code) {

			$container = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
			if (is_null($container)) continue;

			// only list what the current user has access to
			if ($container->readAccess()) {
				if (method_exists($container, 'getContentType')) {
					$content_type = $container->getContentType();
					// recursion check to prevent snippet from nesting itself
					if (!isset($content_type->id) || $content_type->id == $this->id) continue;
					$content_type_type = $content_type->type();
					if (($content_type_type == $this->type()) && $content_type->isEmpty()) {
						// exclude snippet that has not been initialised
						continue;
					}

					// special interface for nest content type
					if($content_type_type == 'content_type_nest_content') {
						$content = $content_type->getSnippetInterface();
					} else {
						// grab html source code of other content types
						$content = $content_type->attr('html');
					}

					$results[$id] = Array(
										'name'		=> $container->name,
										'content'	=> $content,
										'type'		=> $content_type_type,
									);
				}
				unset($content_type);
			}

			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($container);

		}//end foreach container
		return $results;

	}//end getSnippets()


	/**
	* Get a reference to the snippet asset associated with this content type
	*
	* @return mixed object|NULL
	* @access public
	*/
	function &getCurrentSnippet()
	{
		$snippet_asset = NULL;
		$snippet_link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, '', TRUE, 'snippet_asset');
		if (!empty($snippet_link)) {
			$snippet_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($snippet_link['minorid'], $snippet_link['minor_type_code'], TRUE);
		}

		return $snippet_asset;

	}//end getCurrentSnippet()


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
		// provide a more descriptive message when user trash active snippet
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		if ($link['link_type'] == SQ_LINK_NOTICE && $link['value'] == 'snippet_asset') {
			return translate('Linked as an active snippet');

		}
		return parent::describeLink($linkid);

	}//end describeLink()


	/**
	* Returns the editable content of this asset
	*
	* @param boolean $empty_content If TRUE, empty content will be returned
	*
	* @return string
	* @access public
	*/
	function getContent($empty_content=TRUE)
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
	* Perform any additional processing required during the cloning of this asset
	*
	* This function is being called by asset_manager after the dependants of asset
	* have been cloned.
	*
	* @param Asset	$clone		the clone asset shell
	* @param array	$components	the wanted components to clone eg.
	*							Array(
	*							'attributes',
	*							'metadata_schemas',
	*							'metadata',
	*							'workflow',
	*							'permissions',
	*							'data',
	*							);
	*							or alternately
	*							Array('all');
	*
	* @return boolean
	* @access public
	*/
	public function cloneComponentsAdditional(Asset $clone, Array $components)
	{
		// update content.php file
		$bc_container_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($clone->id, SQ_LINK_TYPE_2, 'bodycopy_container', FALSE, 'minor');
		if (!empty($bc_container_link)) {
			$container = $GLOBALS['SQ_SYSTEM']->am->getAsset($bc_container_link[0]['majorid']);
			$edit_fns = $container->getEditFns();
			$edit_fns->generateContentFile($container);
		}

		return parent::cloneComponentsAdditional($clone, $components);

	}//end cloneComponentsAdditional()


}//end class

?>
