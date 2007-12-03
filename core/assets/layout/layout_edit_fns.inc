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
* $Id: layout_edit_fns.inc,v 1.7 2007/12/03 01:16:21 lwright Exp $
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
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Layout_Edit_Fns extends Bodycopy_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* print into a layout edit content a link to the limbo keywords finder
	*
	* @param Layout				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend_Outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintLimboKeywords(Layout $asset, Backend_Outputter $o, $prefix)
	{
	  ?>
	  <p>Click here for <a href="#" onClick="javascript: var win = window.open('<?php echo sq_web_path('data')?>/asset_types/layout/popups/print_limbo_keywords.php', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=520')">a list of specific asset keywords available for use in Simple Edit Mode</a>.</p>
	  <?php

	}//end paintLimboKeywords()


	/**
	* just return TRUE
	*
	* @param Layout				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend_Outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processLimboKeywords(Layout $asset, Backend_Outputter $o, $prefix)
	{
	  return TRUE;

	}//end processLimboKeywords()


	/**
	* Paint the interface for creating containers in the bodycopy
	*
	* @param Layout				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend_Outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintLayout(Layout $asset, Backend_Outputter $o, $prefix)
	{
	  return parent::paintBodycopy($asset, $o, $prefix);

	}//end paintLayout()


	/**
	* Processes the interface for creating containers in the bodycopy
	*
	* @param Layout				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend_Outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processLayout(Layout $asset, Backend_Outputter $o, $prefix)
	{
	  return parent::processBodycopy($asset, $o, $prefix);

	}//end processLayout()


}//end class

?>
