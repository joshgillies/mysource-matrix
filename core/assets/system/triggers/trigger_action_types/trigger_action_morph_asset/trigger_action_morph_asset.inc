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
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';

/**
* Trigger Action Morph Asset
*
* A trigger action that morphs am asset to another type
*
*
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Morph_Asset extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'from_type_code'     => string  // type code of original asset
	*       'to_type_code'    => string  // the type code to morph to
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		// check required settings
		if (!isset($settings['from_type_code']) || empty($settings['from_type_code'])) return FALSE;
		if (!isset($settings['to_type_code']) ||empty($settings['to_type_code'])) return FALSE;
		if (!isset($settings['enforce_uc_status'])) $settings['enforce_uc_status'] = TRUE;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		if (is_null($state['asset'])) return FALSE;
		$asset = $state['asset'];

		// check the optional settings, assign defaults
		$from_type_code = array_get_index($settings, 'from_type_code');
		$to_type_code = array_get_index($settings, 'to_type_code');

		$success = FALSE;
		if ($asset->status == SQ_STATUS_UNDER_CONSTRUCTION || !$settings['enforce_uc_status']) {
			if ($asset->type() === $from_type_code) {
			    if ($asset->morph($to_type_code)) $success = TRUE;
			}
		}

		// if we are not forcing the user asset to be UC then we will have to
		// update the user type stored on the session. This will help the user
		// refresh the session when the setupUser() is called again and the user
		// will not have to logout and log back in again to get the new permissions
		if ($success && !$settings['enforce_uc_status'] && $asset instanceof User && (isset($_SESSION['userid']) && $_SESSION['userid'] == $asset->id)) {
			$_SESSION['user_type_code'] = $to_type_code;
		}

		return $success;

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		// check settings, set defaults if necessary
		$from_type_code    = array_get_index($settings, 'from_type_code', 'user');
		$to_type_code      = array_get_index($settings, 'to_type_code', NULL);
		$enforce_uc_status = array_get_index($settings, 'enforce_uc_status', 1);

		// only user and file type assets support morphing
		$asset_types = Array('user', 'file');
		$asset_types = array_merge($asset_types, $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('user', TRUE));
		$asset_types = array_merge($asset_types, $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants('file', TRUE));
		$from_options = Array();
		foreach ($asset_types as $type_code) {
			$info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
			if ($info['allowed_access'] != 'system' && $info['instantiable']) {
				$from_options[$type_code] = $info['name'];
			}
		}

		ob_start();
		?>
			<p class="sq-backend-section-subheading"><?php echo translate('Morph Settings'); ?></p>
			<table class="sq-backend-table">
				<tr>
					<th style="width:200px">
						<?php
							echo translate('Morph From Type');
						?>
					</th>
					<td>
						<?php
							if ($write_access) {
								combo_box($prefix.'[from_type_code]', $from_options, FALSE, $from_type_code);
							} else {
								echo '<b>'.get_asset_type_icon($from_type_code).$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($from_type_code, 'name').'</b>';
							}
							echo '<div class="sq-backend-note">'.translate('The target asset must match the "morph from" type code.').'</div>';
						?>
					</td>
				</tr>
				<tr>
					<th>
						<?php
		    				echo translate('Morph To Type');
						?>
					</th>
					<td>
						<?php
							if ($write_access) {
								$am = $GLOBALS['SQ_SYSTEM']->am;
								$user = $am->getAsset($GLOBALS['SQ_SYSTEM']->currentUserid());

								$types = array_merge($am->getTypeAncestors($from_type_code), $am->getTypeDescendants($from_type_code));
								for ($i = 0; $i < count($types); $i++) {
									$types[$i] = '\''.$types[$i].'\'';
								}
								$type_code_cond = 'IN ('.implode(', ', $types).')';

								$user_types = $am->getTypeAncestors($user->type());
								$user_types[] = $user->type();
								for ($i = 0; $i < count($user_types); $i++) {
									$user_types[$i] = '\''.$user_types[$i].'\'';
								}
								$user_type_cond = 'IN ('.implode(', ', $user_types).')';

								$sql = 'SELECT type_code
										FROM sq_ast_typ
										WHERE allowed_access '.$user_type_cond.'
										  AND type_code '.$type_code_cond;

								try {
									$query = MatrixDAL::preparePdoQuery($sql);
									$result = MatrixDAL::executePdoAssoc($query, 0);
								} catch (Exception $e) {
									throw new Exception('Unable to get type codes for these user types: '.$user_types.' due to database error: '.$e->getMessage());
								}

								if (empty($result)) {
									echo translate('<i>No types are available to morph this asset into</i>');

								} else {
									$to_options = Array('' => '');
									foreach ($result as $type_code) {
										$code_name = str_replace('_', ' ', $type_code);
										$code_name = ucwords($code_name);
										$to_options[$type_code] = $code_name;
									}
									 combo_box($prefix.'[to_type_code]', $to_options, FALSE, $to_type_code);
								}
							} else {
								if ($to_type_code) {
									echo '<b>'.get_asset_type_icon($to_type_code).$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($to_type_code, 'name').'</b>';
								} else {
									echo translate('<i>No asset type to morph to has been selected.</i>');
								}
							}
						?>
					</td>
				</tr>
				<tr>
					<th>
						<?php
		    				echo translate('Enforce Under Construction Status?');
						?>
					</th>
					<td>
						<?php
							$boolean_options = Array(0 => translate('No'), 1 => translate('Yes'));
							if ($write_access) {
								combo_box($prefix.'[enforce_uc_status]', $boolean_options, FALSE, $enforce_uc_status, 0);
							} else {
								combo_box($prefix.'[enforce_uc_status]', $boolean_options, FALSE, $enforce_uc_status, 0, 'disabled="disabled"');
							}
							echo '<div class="sq-backend-note">'.translate('If set to "Yes", the target asset must be in Under Construction status before it can be morphed.').'</div>';
						?>
					</td>
				</tr>
			</table>
		<?php

		$content = ob_get_clean();

		return $content;

	}//end getInterface()


	/**
	* Handle the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as specified
	*								by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		// optional fields
		$settings['from_type_code']    = array_get_index($request_data, 'from_type_code', 'user');
		$settings['to_type_code']      = array_get_index($request_data, 'to_type_code', '');
		$settings['enforce_uc_status'] = array_get_index($request_data, 'enforce_uc_status', 1);

		return FALSE;

	}//end processInterface()


}//end class

?>
