<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: page_site_map_edit_fns.inc,v 1.1 2003/10/07 04:01:07 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page_edit_fns.inc';

/**
* Page_Site_Map_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Site_Map_Edit_Fns extends Page_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Page_Site_Map_Edit_Fns()
	{
		$this->Page_Edit_Fns();

		// keep our lock after commiting
		$this->static_screens['details']['force_unlock'] = false;

	}//end constructor


	/**
	* paints the exclude list and the asset finder for adding to the exclude list
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintExcludeList(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess()) return false;

		$curr_list = $asset->attr('exclude_list');
		if (empty($curr_list)) {
			echo '<BR><B>No Assets Currently Excluded</B><BR /><BR />';
		} else {
			?><table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header" width="80">Remove?</td>
						<td class="sq-backend-table-header" width="100%">Asset Name</td>
					</tr>
			<?php
			foreach ($curr_list as $id => $name) {
				?>
				<tr>
					<td class="sq-backend-table-cell"><?php
						check_box($prefix.'_curr_exclude_list[]', $id, false);
				?>
					</td>
					<td class="sq-backend-table-cell">
					<?php
						echo $name;
					?>
					</td>
				</tr>
				<?php
			}
			?></table><BR /><BR /><?php
		}
		echo 'choose an Asset to Exclude<BR />';
		asset_finder($prefix.'_exclude_list');

		return true;
	} // end paintExcludeList()


	/**
	* processes the exclude list and the asset finder for adding to the exclude list
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExcludeList(&$asset, &$o, $prefix)
	{
		$exclude_list = $asset->attr('exclude_list');
		if (isset($_POST[$prefix.'_exclude_list']) && $_POST[$prefix.'_exclude_list']) {

			if (!in_array($_POST[$prefix.'_exclude_list'], $exclude_list)) {
				$info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($_POST[$prefix.'_exclude_list']));
				$name = ((empty($info)) ? '' : $info[$_POST[$prefix.'_exclude_list']]['name']).' ('.$_POST[$prefix.'_exclude_list'].')';
				$exclude_list[$_POST[$prefix.'_exclude_list']] = $name;
			}
		}
		if (isset($_POST[$prefix.'_curr_exclude_list'])) {
			foreach ($_POST[$prefix.'_curr_exclude_list'] as $id) {
				unset($exclude_list[$id]);
			}
		}
		$asset->setAttrValue('exclude_list', $exclude_list);
		return $asset->saveAttributes();
	
	}// end processExcludeList()

}//end class

?>