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
* $Id: template_builder_edit_fns.inc,v 1.4 2004/11/02 22:25:16 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Template_Builder_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Template_Builder_Edit_Fns extends Page_Edit_Fns
{
	

	/**
	* Constructor
	*
	*/
	function Template_Builder_Edit_Fns()
	{
		$this->Page_Edit_Fns();

	}//end Template_Builder_Edit_Fns()

	
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
		
		?><p>These questions are pulled from the fields defined in the Template Bodycopy contained underneath this asset. You need to <b><a href="<?php echo $asset->getBackendHref('contents') ?>">Edit the Template Bodycopy</a></b> to change the questions defined.</p>
		
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

				foreach($fields as $qf) {
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
				</table><?php
				
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
			trigger_error('Could not locate the Template Bodycopy for template builder page "'.$asset->name.'"', E_USER_ERROR);
			return false;
		}

		// redirect to the bodycopy's edit contents screen
		$o->setRedirect($bc->getBackendHref('contents'));

		// fudge the navigation history a little so when they click on the link to ger back to the
		// page it doesnt take them to the edit contents screen and then redirect them again
		$_SESSION['sq_nav_history'][(count($_SESSION['sq_nav_history']) - 1)]['screen'] = 'details';

		return false;

	}//end redirectToFormContents()


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

	
}//end class
?>
