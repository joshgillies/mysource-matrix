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
* $Id: page_account_manager_edit_fns.inc,v 1.2 2004/01/15 05:16:34 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page_templates/page_asset_builder/page_asset_builder_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Account_Manager_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Account_Manager_Edit_Fns extends Page_Asset_Builder_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Page_Standard_Edit_Fns()
	{
		$this->Page_Asset_Builder_Edit_Fns();

	}//end constructor


	/**
	* Paint the interface for choosing the user type that is created
	*
	* @param object Page_Account_Manager	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter		&$o		the outputter class
	* @param string							$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCreateType(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			parent::paintCreateType($asset, $o, $prefix);
		} else {
			$user_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('user');
		
			$options = Array('user' => 'User');
			foreach ($user_types as $type_code) {
				$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
				if ($info['allowed_access'] != 'system' && $info['instantiable']) {
					$options[$type_code] = $info['name'];
				}
			}
			combo_box($prefix.'_create_type', $options, false, $asset->attr('create_type'));
		}

	}//end paintCreateType()


	/**
	* Paint the interface for choosing the location to create new assets
	*
	* @param object Page_Asset_Builder	&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCreateLocation(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$locations = $asset->attr('create_location');

		echo '<p>';
		if (empty($locations)) {
			echo 'No user groups have been specified.';
		} else if (count($locations) == 1) {
			echo 'All new user accounts will be created in the user group ';
			$locationid = array_pop($locations);
			$location = &$GLOBALS['SQ_SYSTEM']->am->getAsset($locationid);
			if (!is_null($location)) {
				echo '"'.$location->name.'" [#'.$location->id.']';
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($location);
			}
			echo ' ';
			check_box($prefix.'_delete_location['.$location->id.']');
			echo ' Delete ?';
		} else {
			?>
			<table class="sq-backend-table">
				<tr>
					<td class="sq-backend-table-header">Group Name</td>
					<?php
					if ($write_access) {
						?><td class="sq-backend-table-header">Delete</td><?php
					}
					?>
				</tr>
				<?php
				foreach ($locations as $locationid => $null) {
					$location = &$GLOBALS['SQ_SYSTEM']->am->getAsset($locationid);
					if (is_null($location)) continue;
					?>
					<tr>
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
		echo '</p>';

		if ($write_access) {
			$o->closeField();
			$o->openField('Add New Group');
			asset_finder($prefix.'_new_create_location', 0, Array('user_group' => 'D'));
		}

		return $write_access;

	}//end paintCreateLocation()


}//end class

?>