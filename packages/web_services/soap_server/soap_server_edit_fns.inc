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
* $Id: soap_server_edit_fns.inc,v 1.6 2012/08/30 00:59:46 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Soap_Server_Edit_Fns
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Soap_Server_Edit_Fns extends Page_Edit_Fns
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
	* Paint the API List choice
	*
	* @param object &$asset asset being painted
	* @param object &$o		backend outputter
	* @param string $prefix prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function paintAPIList(&$asset, &$o, $prefix)
	{
		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		$write_access = $asset->writeAccess('attributes');
		$am = $GLOBALS['SQ_SYSTEM']->am;
		$list_api = $am->getChildren($asset->id, 'soap_api', FALSE);

		$known_api = $asset->attr('list_api');

		if (empty($list_api)) {
			echo 'No APIs installed';
		} else {
			// Define the list with an enable/disable field
			$o->openField('');
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header" width="66%"><?php echo translate('API'); ?></td>

						<td class="sq-backend-table-header" width="33%"><?php echo translate('Enable'); ?></td>

					</tr>
			<?php
			foreach ($list_api as $api_id => $api_value) {

				// Find the current value
				$current_value = FALSE;

				if (isset($known_api[$api_id]))	$current_value = TRUE;

				?>
						<tr>
							<td class="sq-backend-table-cell">
								<?php
								echo get_asset_tag_line($api_id);
								?>
							</td>
							<td class="sq-backend-table-cell">
								<?php
									if ($write_access) {
										check_box($prefix.'_enable['.$api_id.']', $api_value[0]['type_code'], $current_value);
									} else {
										?>&nbsp;
										<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo ($current_value) ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
										<?php
									}
								?>
							</td>
						</tr>
				<?php

				// Unload our temp asset here
				$am->forgetAsset($api_asset);
				unset($api_asset);
			}//end foreach
			?>
			</table>
			<?php

			$o->closeField();
			if ($write_access) {
				$o->openField('');
				$o->closeField();
				$o->openField('Regenerate WSDL');
					check_box($prefix.'_regenerate_wsdl', '1', 1);
					$o->note('Select to regenerate the WSDL file to reflect your new changes');
				$o->closeField();
			}//end if
		}//end else

		return TRUE;

	}//end paintAPIList()


	/**
	* Process the API List choice
	*
	* @param object &$asset asset being painted
	* @param object &$o		backend outputter
	* @param string $prefix prefix for the html doc element name
	*
	* @return boolean
	* @access public
	*/
	function processAPIList(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();

		$regen_wsdl = FALSE;
		if ($write_access) {
			if (isset($_POST[$prefix.'_regenerate_wsdl'])) {
				$regen_wsdl = TRUE;
			}//end if
			if (isset($_POST[$prefix.'_enable']) && !empty($_POST[$prefix.'_enable'])) {
				$save_api_list = Array();
				foreach ($_POST[$prefix.'_enable'] as $assetid => $enabled) {
					if (empty($assetid)) continue;

					// Save the entry
					$save_api_list[$assetid] = $enabled;
				}//end foreach

				if (!empty($save_api_list)) {
					$regen_wsdl = TRUE;
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$asset->setAttrValue('list_api', $save_api_list);
					$asset->saveAttributes();
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				}//end foreach
			} else {
				// If it is not set then there is nothing enabled.
				$empty_api_list = Array();
				$regen_wsdl = TRUE;
				$asset->setAttrValue('list_api', $empty_api_list);
				$asset->saveAttributes();
			}//end else
		}//end if write access

		if ($regen_wsdl) {
			$asset->generateWSDL();
		}//end if

	}//end processAPIList()


}//end class
?>
