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
* $Id: page_calendar_edit_fns.inc,v 1.2 2004/07/30 06:35:12 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Calendar_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Edit_Fns extends Page_Asset_Listing_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Calendar_Edit_Fns()
	{
		$this->Page_Asset_Listing_Edit_Fns();

	}//end Page_Calendar_Edit_Fns()


	function paintRootNodes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		
		?><table class="sq-backend-table"><?php
		$locations = $asset->attr('root_nodes');
		if (!$write_access) $location_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_values($locations));
		$l = 1;
		
		foreach (array_keys($locations) as $location) {
			?>
			<tr>
				<td class="sq-backend-table-cell"><?php echo ($l == 1) ? '<b>Root Node(s)</b>' : ''; ?></td>
				<td class="sq-backend-table-cell" colspan="3">
				<?php
					if ($write_access) {
						asset_finder($prefix.'_root_node['.$l.']', $location);
					} else {
						echo $location_info[$location]['name'].' [# '.$location.']';
					}
				?>
				</td>
			</tr>
			<?php
			$l++;
			
		}//end foreach
				
		if ($write_access) {
			?>
			<tr>
				<td class="sq-backend-table-cell"><b>New Root Node?</b></td>
				<td class="sq-backend-table-cell" colspan="3"><?php asset_finder($prefix.'_root_nodes['.$l.']', 0); ?></td>
			</tr>
			<?php
		}
		?></table><?php
		return true;
		
	}//end paintRootNodes()
	
	
	function processRootNodes(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_root_nodes'])) {
			foreach ($_POST[$prefix.'_root_nodes'] as $loc) {
				$locid = $loc['assetid'];
				if (!$locid) continue;
				$locations[$locid] = Array();
			}
		} else {
			return false;
		}
		
		return $asset->setAttrValue('root_nodes', $locations);
	
	}//end processRootNodes()
	
	
}//end class

?>