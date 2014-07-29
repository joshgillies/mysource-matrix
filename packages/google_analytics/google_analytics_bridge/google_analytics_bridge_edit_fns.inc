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
* $Id: data_record_edit_fns.inc,v 1.4 2012/08/30 01:09:05 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/oauth2/oauth2_token/oauth2_token_edit_fns.inc';

/**
* Data_Record_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Google_Analytics_Bridge_Edit_Fns extends OAuth2_Token_Edit_Fns
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
	 * Paints a list of views and a method to create a view.
	 *
	 * @param Google_Analytics_Bridge $asset  The asset being painted.
	 * @param Backend_Outputter       $o      The relevant outputter.
	 * @param string                  $prefix The prefix for form elements.
	 *
	 * @return boolean
	 */
	public function paintViewList(Google_Analytics_Bridge $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$properties   = $asset->getProperties();
		$views        = $asset->getViews();

		if ($write_access === TRUE) {
			$o->openField(translate('Create New View Asset'));
		} else {
			$o->openField(translate('Available Views'));
		}

		echo '<table class="sq-backend-table">';
		if ($write_access === TRUE) {
			echo '<col width="5%" />';
		}
		echo '<col width="50%" />';

		if ($write_access === TRUE) {
			echo '<col width="45%" />';
		} else {
			echo '<col width="50%" />';
		}

		echo '<thead>';
		echo '<tr>';

		if ($write_access === TRUE) {
			echo '<th></th>';
		}

		echo '<th>'.translate('View Name').'</th>';
		echo '<th>'.translate('Property ID').'</th>';
		echo '</tr>';
		echo '</thead><tbody>';

		foreach ($views as $view) {
			$propertyid = $view['sq:propertyId'];
			echo '<tr>';
			if ($write_access === TRUE) {
				echo '<td>';
				radio_button($prefix.'_create_view', $view['id'], FALSE);
				echo '</td>';
			}

			echo '<td><strong>'.$view['name'].'</strong></td>';
			echo '<td>'.$properties[$propertyid]['name'].' ('.$propertyid.')</td>';
			echo '</tr>';
		}//end foreach

		echo '</tbody></table>';
		$o->closeField();

		if ($write_access === TRUE) {
			$o->openField(translate('View Asset Name'));
			text_box($prefix.'_create_view_name', '', 30);
			$o->closeField();
		}

		return $write_access;

	}//end paintViewList()


	/**
	 * Processes a list of views and a method to create a view.
	 *
	 * @param Google_Analytics_Bridge $asset  The asset being painted.
	 * @param Backend_Outputter       $o      The relevant outputter.
	 * @param string                  $prefix The prefix for form elements.
	 *
	 * @return boolean
	 */
	public function processViewList(Google_Analytics_Bridge $asset, Backend_Outputter $o, $prefix)
	{
		$ok        = TRUE;
		$viewid    = array_get_index($_POST, $prefix.'_create_view', '');
		$view_name = array_get_index($_POST, $prefix.'_create_view_name', '');

		if ($viewid !== '') {
			$views = $asset->getViews();
			if (array_key_exists($viewid, $views) === TRUE) {
				// It exists, we can proceed.
				$view = $views[$viewid];
				if ($view_name === '') {
					$view_name = $view['name'];
				}

				$GLOBALS['SQ_SYSTEM']->am->includeAsset('google_analytics_view');
				$view_asset = new Google_Analytics_View();
				$view_asset->setAttrValue('name', $view_name);
				$view_asset->setAttrValue('short_name', $view_name);
				$view_asset->setAttrValue('viewid', $viewid);
				$view_asset->saveAttributes();
				$initial_link = Array(
					'asset'     => $asset,
					'link_type' => SQ_LINK_TYPE_1,
				);
				$view_asset->create($initial_link);

				// Create a notice link to the bridge asset so it can remain linked
				// regardless of whether it is moved
				$notice_link = $GLOBALS['SQ_SYSTEM']->am->createAssetLink($asset, $view_asset, SQ_LINK_NOTICE);
			}//end if (view exists)
		}//end if (view ID provided)

		return $ok;

	}//end processViewList()

}//end class

?>
