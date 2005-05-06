<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: page_template_builder_edit_fns.inc,v 1.8 2005/05/06 01:24:03 tbarrett Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Page_Template_Builder_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Template_Builder_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Page_Template_Builder_Edit_Fns()
	{
		$this->Page_Edit_Fns();

	}//end constructor


	/**
	* Paint the 'Question Fields' field
	*
	* @param object Template_Builder	&$asset
	* @param object Backend_Outputter	&$o
	* @param string						$prefix
	*
	* @return boolean
	* @access public
	*/
	function paintQuestionFields(&$asset, &$o, $prefix)
	{
		$fields = $asset->getQuestionFields();
		?>

		<p>To define the template this asset uses to build bodycopies, you need to 
		<?php if (SQ_IN_BACKEND) echo '<b><a href="'.$asset->getBackendHref('contents').'">'; ?>
		edit the template bodycopy
		<?php if (SQ_IN_BACKEND) echo '</a></b>'; ?>.
		Within that bodycopy you can define question fields using a syntax best described by the following examples:
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-cell" style="font-size: 1em">
					<pre>#summary of symptoms#</pre>
				</td>
				<td class="sq-backend-table-cell">
					A field named "summary of symptoms", whose value will be entered into a 1-line text box.
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="font-size: 1em">
					<pre>#description of problem(30x6)#</pre>
				</td>
				<td class="sq-backend-table-cell">
					A field named "description of problem", whose value will be entered into a text area 30 characters wide and 6 lines high.
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="font-size: 1em">
					<pre>#wall colour[green|light blue|red]#</pre>
				</td>
				<td class="sq-backend-table-cell">
					A field whose value will be chosen from one of the options green, light blue or red.  (In this case the options will be presented on the form as a dropdown box.  However, if the text in the options were longer, the question would be formatted using radio button options).
				</td>
			</tr>
			<tr>
				<td class="sq-backend-table-cell" style="font-size: 1em">
					<pre>#languages{English|Russian|Mandarin}#</pre>
				</td>
				<td class="sq-backend-table-cell">
					A field which may contain several of the options listed.  If several values are chosen they will be concatenated by commas and the word 'and', for example 'English, Russian and Mandarin'.
				</td>
			</tr>
		</table>
		</p>

		<p>The fields defined in the current template bodycopy are listed below:
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">
					Question Field Name
				</td>
				<td class="sq-backend-table-header">
					Type
				</td>
				<td class="sq-backend-table-header">
					Possible Values
				</td>
				<?php

				foreach ($fields as $qf) {
					?>
						<tr>
							<td class="sq-backend-table-cell">
								<?php echo $qf['name'] ?>
							</td>
							<td class="sq-backend-table-cell">
								<?php
								switch ($qf['type']) {
									case '[':
										echo 'Select';
									break;

									case '{':
										echo 'Multiple Select';
									break;

									case '(':
									case '':
										echo 'Freeform Text';
									break;

									default:
										echo '* Unknown *';
									break;

								}//end switch
							?></td>
							<td class="sq-backend-table-cell">
								<?php
								if ($qf['type'] == '(') {
									echo $qf['values'][0].' column(s) x '.$qf['values'][1].' row(s)';
								} else if ($qf['type'] == '') {
									// default for freeform text
									echo 'Default: 40 column(s) x 1 row(s)';
								} else {
									echo implode(', ', $qf['values']);
								} ?>
							</td>
						</tr>
					<?php
				}//end foreach
				?>
				</table></p>
				<?php

		return true;

	}//end paintQuestionFields()


	/**
	* Process Question Fields field - no processing done here
	*
	* @return boolean	true always
	*/
	function processQuestionFields(&$asset, &$o, $prefix)
	{
		return true;

	}//end processQuestionFields()


	/**
	* Redirect to the template bodycopy
	*
	* @param object Template_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function redirectToTemplateBodycopy(&$asset, &$o, $prefix)
	{
		// get the bodycopy attached to us
		$bc_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy', false, 'major', 'template');
		$bc = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bc_link[0]['minorid']);
		if (is_null($bc)) {
			trigger_localised_error('CMS0015', E_USER_ERROR, $asset->name);
			return false;
		}

		// redirect to the bodycopy's edit contents screen
		$o->setRedirect($bc->getBackendHref('contents'));

		// fudge the navigation history a little so when they click on the link to ger back to the
		// page it doesnt take them to the edit contents screen and then redirect them again
		$_SESSION['sq_nav_history'][(count($_SESSION['sq_nav_history']) - 1)]['screen'] = 'details';

		return false;

	}//end redirectToTemplateBodycopy()


	//--        CREATE LOCATIONS SCREEN         --//


	/**
	* Paint the interface for choosing the locations to which all new assets will be linked
	*
	* @param object Template_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintFixedCreateLocations(&$asset, &$o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		?><p><b>Note:</b> all links to newly created bodycopies will be of TYPE_2; that is, they will appear in the asset map of the Administration interface, but not in navigation (menus, etc.) on the frontend.</p><?php

		$write_access = $asset->writeAccess('attributes');
		$locations = $asset->attr('create_locations');

		if (empty($locations)) {
			echo 'No fixed create locations have been specified';
		} else {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">ID</td>
					<td class="sq-backend-table-header">Name</td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header">Delete</td><?php
					}
					?>
				</tr>
				<?php

				foreach ($locations as $locationid => $link_type) {
					$location = &$GLOBALS['SQ_SYSTEM']->am->getAsset($locationid);
					if (is_null($location)) continue;
					?>
					<tr>
						<td class="sq-backend-table-cell"><?php echo $location->id; ?></td>
						<td class="sq-backend-table-cell"><?php echo $location->name; ?></td>
						<?php
						if ($write_access) {
							?><td class="sq-backend-table-cell"><?php check_box($prefix.'_delete_location['.$location->id.']'); ?></td><?php
						}
						?>
					</tr>
					<?php
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($location);
				}
				?>
			</table>
			<?php
		}

		if ($write_access) {
			echo '<p><b>Add Location:</b>&nbsp;&nbsp;&nbsp; ';
			asset_finder($prefix.'_new_create_location', 0);
			echo '</p>';
		}

		return $write_access;

	}//end paintFixedCreateLocations()


	/**
	* Processes the interface for choosing the locations to which all new assets will be linked
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processFixedCreateLocations(&$asset, &$o, $prefix)
	{
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';

		$old_locations = $asset->attr('create_locations');
		$new_locations = $old_locations;

		// add a new create location - all links in the template builder are TYPE_2
		if (isset($_POST[$prefix.'_new_create_location'])) {
			$new_location =(int) $_POST[$prefix.'_new_create_location']['assetid'];
			if ($new_location && !isset($new_locations[$new_location])) {
				$new_locations[$new_location] = SQ_LINK_TYPE_2;
			}
		}

		// delete existing create locations
		if (isset($_POST[$prefix.'_delete_location'])) {
			foreach ($_POST[$prefix.'_delete_location'] as $locationid => $on) {
				if (isset($new_locations[$locationid])) unset($new_locations[$locationid]);
			}
		}

		if ($old_locations != $new_locations) return $asset->setAttrValue('create_locations', $new_locations);
		return false;

	}//end processFixedCreateLocations()



	/**
	* Paints the interface for saying where additional create locations can come from
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintAdditionalLocationParents(&$asset, &$o, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			multiple_asset_finder($prefix.'_additional_location_parents', $asset->attr('additional_location_parents'), Array());
		} else {
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($asset->attr('additional_location_parents'));
			echo '<ul style="margin: 1px 15px;">';
			foreach ($infos as $assetid => $info) {
				echo '<li>'.$info['name'].' (ID #'.$assetid.')</li>';
			}
			echo '</ul>';
		}
		return true;

	}//end paintAdditionalLocationParents()


	/**
	* Processes the interface for saying where additional create locations can come from
	*
	* @param object Page_Asset_Builder	&$asset	the asset to which we belong
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processAdditionalLocationParents(&$asset, &$o, $prefix)
	{
		$new_parents = Array();
		if (isset($_POST[$prefix.'_additional_location_parents']) && is_array($_POST[$prefix.'_additional_location_parents'])) {
			foreach ($_POST[$prefix.'_additional_location_parents'] as $parent) {
				if ($parent['assetid']) $new_parents[] = $parent['assetid'];
			}
		}
		$asset->setAttrValue('additional_location_parents', $new_parents);

	}//end processAdditionalLocationParents()


}//end class
?>
