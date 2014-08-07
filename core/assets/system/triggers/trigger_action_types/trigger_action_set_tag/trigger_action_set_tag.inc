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
* $Id: trigger_action_set_tag.inc,v 1.14 2013/09/26 09:09:18 cupreti Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';

/**
* Trigger Action Set Tag
*
* A trigger action that sets a thesaurus tag on the firing asset
*
*
* @author  Robert Howard <rhoward@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Set_Tag extends Trigger_Action
{


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:
	* <pre>
	* Array(
	* 		'tagid'				=> string,	// the assetid of the tag
	* 		'thesaurusid'		=> string,	// optionally, supply the thesaurus id...
	*		'term_name'			=> string,	// ... and term name instead
	* 		'weighting'			=> int,		// what weighting to the give the new tag
	* 		'action'			=> string,	// whether to add or delete
	* 		'cascade'			=> boolean,	// cascade tag changes to children?
	* 		'manual_cascade'	=> boolean,	// manually cascade the changes instead (mutually exclusive with 'cascade')
	* 		'value_map'			=> serialised array // if a dynamic param is set, extract tag id from here
	* );
	* </pre>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		$result = FALSE;

		if (empty($state['asset'])) {
			// grab the asset if assetid is given, but not the asset.
			if (empty($state['assetid'])) {
				return FALSE;
			} else {
				$state['asset'] = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
			}
		}

		// note that tagid can be an assetid or an array of ids
		$tagid = 0;
		// see if the tag id is set in the parameter map
		$value_map = array_get_index($settings, 'value_map', serialize(Array()));
		$param_map_attr = new Asset_Attribute_Parameter_Map(0, $value_map);
		$param_map_value = $param_map_attr->getParameterValue('tag_id');
		if (!empty($param_map_value)) {
			// check if map value is array, otherwise explode by comma
			if (is_array($param_map_value)) $tmp_ids = $param_map_value;
			else $tmp_ids = explode(',', $param_map_value);
			$tagid = Array();
			foreach ($tmp_ids as $tmp_id) {
				if ($GLOBALS['SQ_SYSTEM']->am->assetExists($tmp_id)) {
					// only add valid tag id to the array
					$tagid[] = $tmp_id;
				} else {
					trigger_localised_error('SYS0087', E_USER_WARNING, $tmp_id);
				}
			}
		} else {
			// if none is set in the param map,
			// use the normal tag id chosen by the asset finder
			if (!empty($settings['tagid']) && $GLOBALS['SQ_SYSTEM']->am->assetExists($settings['tagid'])) {
				$tagid = $settings['tagid'];
			} else {
				// use the thesaurusid / term_name combo instead
				if (!empty($settings['thesaurusid'])) {
					$thesaurus = $GLOBALS['SQ_SYSTEM']->am->getAsset($settings['thesaurusid']);
					if (!is_null($thesaurus)) {
						$termid = $thesaurus->getTermIdByName($settings['term_name']);
						if (!empty($termid)) {
							$tagid = $thesaurus->id.':'.$termid;
						}
						$GLOBALS['SQ_SYSTEM']->am->forgetAsset($thesaurus);
					}
				}
			}
		}
		if (empty($tagid)) return FALSE;

		$action = array_get_index($settings, 'action', 'add');
		if ($action == 'delete') {
			$weighting = NULL;
		} else {
			$weighting = array_get_index($settings, 'weighting', NULL);
		}

		$manual_cascade = array_get_index($settings, 'manual_cascade', FALSE);
		if ($manual_cascade) {
			// can't cascade and manually cascade at the same time
			$cascade = FALSE;
		} else {
			$cascade = array_get_index($settings, 'cascade', FALSE);
		}


		$tag_manager = $GLOBALS['SQ_SYSTEM']->getTagManager();

		$tag_vars = Array();

		switch ($action) {
			case 'add':
				if ($cascade) {
					$tag_vars[] = Array(
									'tag_id'	=> $tagid,
									'action'	=> $action,
									'weight'	=> (!$weighting ? 1 : $weighting),
								  );
				} else {
					$result = $tag_manager->setTag($state['assetid'], $tagid, $weighting);
				}
			break;
			case 'delete':
				if ($cascade) {
					$tag_vars[] = Array(
									'tag_id'	=> $tagid,
									'action'	=> $action,
									'weight'	=> NULL,
								  );
				} else {
					$result = $tag_manager->deleteTag($state['assetid'], $tagid);
				}
			break;
		}

		if ($manual_cascade) {
			// we've already had the tags set on/removed from the asset, now
			// manually propogate all tags on the asset to its children
			$current_tag_links = $tag_manager->getTagLinks($state['assetid']);

			foreach ($current_tag_links as $link) {
				$tag_vars[] = Array(
								'tag_id'	=> $link['minorid'],
								'action'	=> 'add',
								'weight'	=> $link['value'],
							  );
			}
		}

		if (!empty($tag_vars)) {
			$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array(
						'assets'		=> Array(
											$state['asset']->id	=> Array(
																	'type_code'	=> $state['asset']->type(),
																   ),
										   ),
						'tag_changes'	=> $tag_vars,
					);
			$errors = $hh->freestyleHIPO('hipo_job_edit_tags', $vars);
			$result = empty($errors);
		}

		if (!$result) {
			return FALSE;
		} else {
			return Array(
					'assetid'	=> $state['asset']->id,
					'tagid'		=> $tagid,
					'weighting'	=> $weighting,
					'action'	=> $action,
				   );
		}

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
		// print javascript to disable weighting checkbox, and to handle cascading
		ob_start();
			if ($write_access) {
				?><script>
					function toggleWeighting(value) {
						var prefix  = '<?php echo $prefix; ?>';
						var id      = prefix + '[weighting]';
						var element = document.getElementById(id);
						if (!value) {
							element.disabled = 'disabled';
						} else {
							element.disabled = '';
						}
					}

					function toggleExclusiveCheckboxes(check_clicked) {
						var checkboxes = new Array();
						checkboxes[0]  = 'cascade';
						checkboxes[1]  = 'manual_cascade';

						var prefix  = '<?php echo $prefix; ?>';
						var id      = '';
						var ii      = 0;

						for (ii = 0; ii < checkboxes.length; ii++) {
							if (checkboxes[ii] != check_clicked) {
								id = prefix + '[' + checkboxes[ii] + ']';
								document.getElementById(id).checked = false;
							}
						}
					}
				</script><?php
			}

			// check settings, set defaults if necessary
			$tagid     = array_get_index($settings, 'tagid', 0);
			$action    = array_get_index($settings, 'action', 'add');
			$weighting = array_get_index($settings, 'weighting', ($action == 'delete' ? NULL : 1));

			$manual_cascade = array_get_index($settings, 'manual_cascade', FALSE);
			if ($manual_cascade) {
				// can't cascade and manually cascade at the same time
				$cascade = FALSE;
			} else {
				$cascade = array_get_index($settings, 'cascade', FALSE);
			}


			// check if the selected asset is valid
			if ($tagid && !$GLOBALS['SQ_SYSTEM']->am->assetExists($tagid)) {
				$tagid = 0;
			}

			ob_start();
				if ($write_access) {
					asset_finder($prefix.'[tag]', $tagid, Array('thesaurus_term' => 'D'));
					echo '<br />';
				} else {
					if ($tagid) {
						$tag = $GLOBALS['SQ_SYSTEM']->am->getAsset($tagid);
						if (!is_null($tag)) {
							echo get_asset_tag_line($tag->id);
						}
					} else {
						echo '<em>'.translate('trigger_set_tag_no_tag_selected').'</em>';
					}
				}
				$tag_part = ob_get_contents();
			ob_end_clean();

			ob_start();
				if ($write_access) {
					$disabled = ($action == 'add') ? '' : 'disabled="disabled"';
					int_text_box($prefix.'[weighting]', $weighting, TRUE, 5, NULL, NULL, '', FALSE, FALSE, $disabled);
				} else {
					echo '<b>'.$weighting.'</b>';
				}
				$weighting_part = ob_get_contents();
			ob_end_clean();

			ob_start();
				$action_checked = ($action == 'add' ? FALSE : TRUE);
				if ($write_access) {
					echo '<em>'.strtoupper(translate('or')).'</em></br>';
					check_box($prefix.'[action]', '1', $action_checked, 'toggleWeighting(!this.checked)');
				} else {
					?><img src="<?php echo sq_web_path('lib').'/web/images/'.($action_checked ? 'tick' : 'cross').'.gif'; ?>" alt="<?php echo ($action_checked ? translate('yes') : translate('no')); ?>"><?php
				}
				label(translate('trigger_set_tag_label_remove_tag'), $prefix.'[action]');
				$delete_part = ob_get_contents();
			ob_end_clean();

			ob_start();
				if ($write_access) {
					check_box($prefix.'[cascade]', '1', $cascade, 'toggleExclusiveCheckboxes(\'cascade\')');
				} else {
					?><img src="<?php echo sq_web_path('lib').'/web/images/'.($cascade ? 'tick' : 'cross').'.gif'; ?>" alt="<?php echo ($cascade ? translate('yes') : translate('no')); ?>"><?php
				}
				label(translate('cascade_tag_changes'), $prefix.'[cascade]');
				$cascade_part = ob_get_contents();
			ob_end_clean();

			ob_start();
				if ($write_access) {
					check_box($prefix.'[manual_cascade]', '1', $manual_cascade, 'toggleExclusiveCheckboxes(\'manual_cascade\')');
				} else {
					?><img src="<?php echo sq_web_path('lib').'/web/images/'.($manual_cascade ? 'tick' : 'cross').'.gif'; ?>" alt="<?php echo ($manual_cascade ? translate('yes') : translate('no')); ?>"><?php
				}
				label(translate('manually_cascade_all_tags'), $prefix.'[manual_cascade]');
				$manual_cascade_part = ob_get_contents();
			ob_end_clean();

			echo translate('trigger_set_tag_select_tag').' '.$tag_part;
			if ($write_access || $action == 'add') {
				echo '<br />'.translate('trigger_set_tag_set_weighting').' '.$weighting_part;
			}
			echo '<br />';
			echo $delete_part;
			echo '<br /><br />';
			echo '<div class="sq-break-line-inputs">';
			echo  $cascade_part;
			echo '<br />';
			echo  $manual_cascade_part;
			echo  '</div>';

			// prints the parameter map to accept dynamic values
			if ($write_access) {
				// attribute friendly prefix
				$new_prefix = str_replace(array('[',']'), '_', $prefix);
				hidden_field($prefix.'[new_prefix]', $new_prefix);
				$param_map_value = array_get_index($settings, 'value_map', serialize(Array()));
				$param_map_attr = new Asset_Attribute_Parameter_Map(0, $param_map_value);
				$param_map_attr->setParameter('tag_id', 'Tag ID');
				echo $param_map_attr->paint($new_prefix.'_parameter_map');
			}
			$contents = ob_get_contents();
		ob_end_clean();

		return $contents;

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
		$tagid = '';
		if (!empty($request_data['tag']['assetid'])) {
			$tagid = $request_data['tag']['assetid'];
		}

		$settings['tagid']          = $tagid;
		$settings['weighting']      = array_get_index($request_data, 'weighting', NULL);
		$settings['action']         = (array_get_index($request_data, 'action') ? 'delete' : 'add');

		$settings['cascade']        = (bool) array_get_index($request_data, 'cascade', FALSE);
		$settings['manual_cascade'] = (bool) array_get_index($request_data, 'manual_cascade', FALSE);

		// parameter map to accept dynamic value
		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));
		if ($new_prefix) {
			$param_map_attr = new Asset_Attribute_Parameter_Map();
			$param_map_attr->process($new_prefix.'_parameter_map');
			$settings['value_map'] = $param_map_attr->value;
		}

		return FALSE;

	}//end processInterface()


	/**
	* Get the list of locks that the action needs to acquire before executing
	*
	* @param array	$settings	a container for any data the action might want to save
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return array
	* @access public
	*/
	public static function getLocks($settings, &$state)
	{
		return Array(
				$state['assetid']	=> Array('all'),
			   );

	}//end getLocks()


}//end class
?>
