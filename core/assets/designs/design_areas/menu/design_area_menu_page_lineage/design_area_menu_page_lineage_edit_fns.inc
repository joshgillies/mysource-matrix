<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_menu_page_lineage_edit_fns.inc,v 1.5 2003/10/13 07:27:45 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_areas/menu/design_area_menu_type/design_area_menu_type_edit_fns.inc';

/**
* Design_Area_Menu_Page_Lineage_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Menu_Page_Lineage_Edit_Fns extends Design_Area_Menu_Type_Edit_Fns
{

	/**
	* Takes the contents for the sub menu element and attempts to process it, 
	* is a separate fn to allow menu types that don't want to allow sub menu's to override
	*
	* @param object Asset	$asset			the asset to which we belong
	* @param string			$design_area	the menu type design area that the sub menu is supposed to be
	* @param array			$contents		the sub menu's contents array that we generated with _parseString()
	*
	* @return boolean
	* @access private
	* @see Design_Area_Menu_Type_Edit_Fns::_processContents();
	*/
	function _processSubMenu(&$asset, &$contents)
	{
		// we just want to ignore any stupid sub menu's in our page contents
		trigger_error('You are unable to set sub menu\'s on the page lineage menu type', E_USER_WARNING);
		return false;

	}// end _processSubMenu()


	/**
	* Outputs the PHP/HTML code that will hopefully form part of the creating the design file
	*
	* @param Array			$contents	the parse contents to use in the output
	* @param object Asset	$asset		the asset to which we belong
	* @param object Design	$design		the design object that is painting us
	*
	* @return boolean
	* @access private
	*/
	function _createDesignFileOutput($contents, &$asset, &$design)
	{
		$class = ($asset->attr('class')) ? ' class="'.$asset->attr('class').'"' : '';

		// because the page lineage array contains the site index, all we need to do to have or not have
		// the home link is start the loop at a different position
		$start_index = ($asset->attr('prefix_with_home_link')) ? 0 : 1;

		$total = 'COUNT($ASSET_LINEAGE)';
		// because the page lineage array contains the current page, all we need to do to have or not have
		// the current link is end the loop at a different position
		if (!$asset->attr('suffix_with_current_link')) { 
			$total .= ' - 1';
		}

		// if we have pages to print, we just adjust the total count factor
		if ($asset->attr('pages_to_print') > 0) { 
			$total = 'min('.$total.', '.((int) $asset->attr('pages_to_print') + $start_index).')';
		}

		echo '<span', $class,'><', '?php

		require_once SQ_FUDGE_PATH.\'/general/www.inc\';
		$'.$asset->attr('id_name').'_url   = current_url();
		$'.$asset->attr('id_name').'_proto = current_protocol();

		for($'.$asset->attr('id_name').'_i = '.$start_index.', $'.$asset->attr('id_name').'_total = '.$total.'; 
			$'.$asset->attr('id_name').'_i < $'.$asset->attr('id_name').'_total; 
			$'.$asset->attr('id_name').'_i++) {
			';
		if ($asset->attr('prefix_with_divider')) {
			echo '
			if ($'.$asset->attr('id_name').'_i == '.$start_index.') echo '.var_export($asset->attr('divider'), true).';
			';
		}

		if ($asset->attr('have_links')) {
			echo '?','><a', $class, ' href="<','?php echo relative_href($'.$asset->attr('id_name').'_url, $'.$asset->attr('id_name').'_proto.\'://\'.$ASSET_LINEAGE[$'.$asset->attr('id_name').'_i][\'url\']); ?','>"><','?php ';
		}

		echo 'echo htmlspecialchars($ASSET_LINEAGE[$'.$asset->attr('id_name').'_i][\'short_name\']);';

		if ($asset->attr('have_links')) {
			echo '?','></a><','?php ';
		}

		// if there is not divider at the end we need to check that we aren't at the end of the list
		if (!$asset->attr('suffix_with_divider')) {
			echo '
				if ($'.$asset->attr('id_name').'_i != $'.$asset->attr('id_name').'_total - 1) {
				';
		}

		echo '?','>', htmlspecialchars($asset->attr('divider')), '<','?php ';

		// if there is not divider at the end we need to check that we aren't at the end of the list
		if (!$asset->attr('suffix_with_divider')) {
			echo '
				}// end if';
		}

		echo '
		}// end for
		?','></span>';

		return true;

	}// end _createDesignFileOutput()

}//end class
?>
