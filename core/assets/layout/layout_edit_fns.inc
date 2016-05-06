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
* $Id: layout_edit_fns.inc,v 1.10 2013/09/24 01:16:55 ewang Exp $
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
* @version $Revision: 1.10 $
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
	  <p>Click here for <a href="#" onClick="javascript: var win = window.open('<?php echo sq_web_path('lib')?>/web/popups/print_limbo_keywords.php', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=520')">a list of specific asset keywords available for use in Simple Edit Mode</a>.</p>
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


	/**
	* Paint the interface for customising Layout options
	*
	* @param Layout				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend_Outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintLayoutOptions(Layout $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$layout_display_options = $asset->attr('options');

		$show_cascade_metadata_field = FALSE;
		if (isset($layout_display_options['show_cascade_metadata_field'])) {
			$show_cascade_metadata_field = $layout_display_options['show_cascade_metadata_field'];
		}

		if ($write_access) {
			check_box($prefix.'[show_cascade_metadata_field]', 1, $show_cascade_metadata_field);
			label(translate('Show "Cascade Metadata" field'), $prefix.'[show_cascade_metadata_field]');

		} else {
			?>
				<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($show_cascade_metadata_field ? 'tick' : 'cross'); ?>.png" width="15" height="15" />
			<?php
			echo translate('Show "Cascade Metadata" field');

		}

		return TRUE;

	}//end paintLayoutOptions()


	/**
	* Processes the interface for Layout options
	*
	* @param Layout				$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the Backend_Outputter class
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processLayoutOptions(Layout $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {
			$layout_display_options = $asset->attr('options');

			// With checkboxes, default to unset unless it is set, in which case set the value. Now we're all set
			$layout_display_options['show_cascade_metadata_field'] = FALSE;

			if (isset($_POST[$prefix]['show_cascade_metadata_field'])) {
				$layout_display_options['show_cascade_metadata_field'] = (bool) $_POST[$prefix]['show_cascade_metadata_field'];
			}

			$asset->setAttrValue('options', $layout_display_options);
			$asset->saveAttributes();
		}

		return TRUE;

	}//end processLayoutOptions()


}//end class

?>
