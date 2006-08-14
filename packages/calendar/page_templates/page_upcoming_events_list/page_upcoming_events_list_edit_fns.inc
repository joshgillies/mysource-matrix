<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: page_upcoming_events_list_edit_fns.inc,v 1.1 2006/08/14 03:20:28 tbarrett Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';


/**
* Page_Upcoming_Events_List_Edit_Fns
*
* Purpose
*	Functions to help print and process the backend interface of rolling calendar page assets
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Upcoming_Events_List_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Upcoming_Events_List_Edit_Fns()
	{
		$this->Page_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for setting the root nodes from which this calendar page draws events
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintEventSources(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			multiple_asset_finder($prefix.'_event_sources', array_keys($asset->attr('root_nodes')));
			return TRUE;
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($asset->attr('root_nodes')), '', TRUE, 'name');
			?><ul style="margin: 1px 15px;"><?php
			foreach ($infos as $id => $name) {
				echo '<li>'.get_asset_tag_line($id).'</li>';
			}
			?></ul><?php
			return FALSE;
		}

	}//end paintEventSources()


	/**
	* Process the interface for setting the root nodes from which this calendar page draws events
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processEventSources(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_event_sources'])) {
			$root_nodes = Array();
			foreach ($_POST[$prefix.'_event_sources'] as $source) {
				if (!empty($source['assetid'])) {
					$root_nodes[$source['assetid']] = 1;
				}
			}
			return $asset->setAttrValue('root_nodes', $root_nodes);
		}
		return FALSE;

	}//end processEventSources()


}//end class
?>