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
* $Id: layout_edit_fns.inc,v 1.6 2007/02/05 05:39:05 colivar Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy/bodycopy_edit_fns.inc';

/**
* Layout_Edit_Fns
*
* Purpose
*
*
* @author Marc McIntyre <mmcintyre@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Layout_Edit_Fns extends Bodycopy_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Layout_Edit_Fns()
	{
		$this->Bodycopy_Edit_Fns();

	}//end constructor


	/**
	 * print into a layout edit content a link to the limbo keywords finder
	 *
	 * @param object &$asset the asset whose interface we are painting
	 * @param object &$o             the Backend_Outputter class
	 * @param string $prefix prefix for the form elements
	 *
	 * @return boolean
	 * @access public
	 */
	function paintLimboKeywords(&$asset, &$o, $prefix)
	{
	  ?>
	  <p>Click here for <a href="#" onClick="javascript: var win = window.open('<?php echo sq_web_path('data')?>/asset_types/layout/popups/print_limbo_keywords.php', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=520')">a list of specific asset keywords available for use in Simple Edit Mode</a>.</p>
	  <?php

	}//end paintLimboKeywords()


	/**
	 * just return TRUE
	 *
	 * @param object &$asset the asset whose interface we are painting
	 * @param object &$o             the Backend_Outputter class
	 * @param string $prefix prefix for the form elements
	 *
	 * @return boolean
	 * @access public
	 */
	function processLimboKeywords(&$asset, &$o, $prefix)
	{
	  return TRUE;

	}//end processLimboKeywords()


	/**
	 * Paint the interface for creating containers in the bodycopy
	 *
	 * @param object &$asset the asset whose interface we are painting
	 * @param object &$o             the Backend_Outputter class
	 * @param string $prefix prefix for the form elements
	 *
	 * @return boolean
	 * @access public
	 */
	function paintLayout(&$asset, &$o, $prefix)
	{
	  return parent::paintBodycopy($asset, $o, $prefix);

	}//end paintLayout()


	/**
	 * Processes the interface for creating containers in the bodycopy
	 *
	 * @param object &$asset the asset to which we belong
	 * @param object &$o             the Backend_Outputter class
	 * @param string $prefix prefix for the form element
	 *
	 * @return boolean
	 * @access public
	 */
	function processLayout(&$asset, &$o, $prefix)
	{
	  return parent::processBodycopy($asset, $o, $prefix);

	}//end processLayout()


}//end class

?>
