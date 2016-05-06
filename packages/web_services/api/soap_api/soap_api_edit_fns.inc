<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: soap_api_edit_fns.inc,v 1.5 2012/10/03 05:56:12 akarelia Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Soap_Api_Edit_Fns
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class SOAP_Api_Edit_Fns extends Asset_Edit_Fns
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
	* Paint the Function List choice
	*
	* @param object &$asset asset being painted
	* @param object &$o		backend outputter
	* @param string $prefix prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintFunctionList(&$asset, &$o, $prefix)
	{
        $am = $GLOBALS['SQ_SYSTEM']->am;
        $admin_access = $asset->writeAccess('attributes');
        $prefix = $asset->getPrefix();
		$enabled_function_list = $asset->attr('function_list');
		$function_list = $asset->getFunctionList();
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">
					<?php echo translate('Operations'); ?>

				</td>
				<td class="sq-backend-table-header">
					<?php echo translate('Enabled'); ?>

				</td>
			</tr>
		<?php
		foreach ($function_list as $func_name => $default) {
			$switch = FALSE;
			if (array_key_exists($func_name, $enabled_function_list)) {
				if (isset($enabled_function_list[$func_name]) && $enabled_function_list[$func_name]) {
					$switch	= TRUE;
				}
			}
		?>
			<tr>
				<td class="sq-backend-table-cell">
					<span name="<?php echo $prefix.'_'.$func_name ?>" id="<?php echo   $prefix.'_'.$func_name ?>"><?php echo $func_name ?></span>
				</td>
				<td align="left" class="sq-backend-table-cell">
		<?php
					if ($admin_access) {
						check_box($prefix.'_enable['.$func_name.']', '1', $switch);
					} else {
		?>
						&nbsp;
						<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($switch) ? 'tick' : 'cross'; ?>.png" width="15" height="15" />
		<?php
					}
		?>
				</td>
			</tr>
		<?php
		}//end foreach
		?>
		</table>
		<?php

		return TRUE;

	}//end paintFunctionList()


	/**
	* Process the Function List choice
	*
	* @param object &$asset asset being painted
	* @param object &$o		backend outputter
	* @param string $prefix prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processFunctionList(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();

		$regen_wsdl = FALSE;
		if ($write_access) {
			if (isset($_POST[$prefix.'_enable']) && !empty($_POST[$prefix.'_enable'])) {
				$save_func_list = Array();
				foreach ($_POST[$prefix.'_enable'] as $funcname => $enabled) {
					if (empty($funcname)) continue;

					// Save the entry
					$save_func_list[$funcname] = $enabled;
				}

				if (!empty($save_func_list)) {
					$regen_wsdl = TRUE;
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$asset->setAttrValue('function_list', $save_func_list);
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				}
			} else {
				// If it is not set then there is nothing enabled.
				$empty_function_list = Array();
				$regen_wsdl = TRUE;
				$asset->setAttrValue('function_list', $empty_function_list);
				$asset->saveAttributes();
			}//end else
		}
		if ($regen_wsdl) {
			$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($asset->id, 'soap_server');
			foreach ($parents as $parent_id => $type_code) {
				$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_id);
				$parent->generateWSDL();
			}//end foreach
			//$asset->generateWSDL();
		}//end if

	}//end processFunctionList()


}//end class
?>
