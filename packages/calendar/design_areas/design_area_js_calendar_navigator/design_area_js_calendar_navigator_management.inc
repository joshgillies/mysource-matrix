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
* $Id: design_area_js_calendar_navigator_management.inc,v 1.10 2012/08/30 00:57:28 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_JS_Calendar_Navigator_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Design_Area_JS_Calendar_Navigator_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Area_JS_Calendar_Navigator_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'target_assetid'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'DEPRECATED - DONE WITH LINKS INSTEAD FROM V0.2',
											   ),
						'element_id'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> 'js_calendar_navigator',
												'description'	=> 'Element ID on HTML page',
											   ),
						'style_assetid'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'Optional CSS',
											   ),
					  );

	}//end constructor


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		if (version_compare($current_version, '0.2', '<')) {
			pre_echo('UPGRADING CALENDAR NAVIGATOR DESIGN AREA TO V0.2');
			$instances = $am->getTypeAssetids($this->getAssetType(), FALSE, TRUE);
			$to_reparse = Array();
			foreach ($instances as $id => $type) {
				$instance = $am->getAsset($id, 'design_area_js_calendar_navigator');
				$pageid = @$instance->attr('target_assetid');
				if (!empty($pageid)) {
					$page = $am->getAsset($pageid);
					if (!is_null($page)) {
						foreach (Array('day', 'week', 'month', 'year') as $view) {
							$instance->createLink($page, SQ_LINK_NOTICE, $view.'_page');
						}
					}
				}
				$designs = $instance->getDesigns();
				foreach (array_keys($designs) as $i) {
					if ($designs[$i]->type() == 'design') {
						// only want real designs, customisations will look after themselves
						$to_reparse[] = $designs[$i]->id;
					}
				}
			}
			pre_echo('REPARSING DESIGNS');
			$edit_fns = NULL;
			foreach (array_unique($to_reparse) as $designid) {
				$design = $am->getAsset($designid, 'design');
				if (is_null($edit_fns)) {
					$edit_fns = $design->getEditFns();
				}
				$edit_fns->parseAndProcessFile($design);
				$design->_updateCustomisations();
			}
			pre_echo('CALENDAR NAVIGATOR DESIGN AREA COMPLETE');
		}//end if version less than 0.2
		return TRUE;

	}//end _upgrade()


}//end class
?>
