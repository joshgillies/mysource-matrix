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
* $Id: page_asset_builder.inc,v 1.27 2004/09/03 17:47:15 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Page_Asset_Builder
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Version$ - 1.4
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Builder extends Page
{

	/**
	* The current state the asset builder is in
	* Used for determining which bodycopy to use for printing
	* @var string
	*/
	var $_current_state = '';

	/**
	* An array of errors reported during execution
	* @var Array()
	*/
	var $_errors = Array();

	/**
	* An array of keyword replacements to make
	* @var Array()
	*/
	var $_replacements = Array();

	/**
	* An array of keywords that this asset makes available for use along with a default replacements
	* @var Array()
	*/
	var $_available_keywords = Array(
								'login_invite',
								'login_form',
								'login_error',
								'create_invite',
								'create_form',
								'create_error',
								);

	/**
	* An array of bodycopies that need to be created
	* @var Array()
	*/
	var $_bodycopies = Array('Not Logged In' => 'not_logged_in',
							 'Created'       => 'created',
							 'Logged In'     => 'logged_in',
							);

	/**
	* The default state the system should be in if the user is logged in
	* @var string
	*/
	var $_default_state = 'create';


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Page_Asset_Builder($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Page($assetid);

	}//end constructor


	/**
	* Create this asset
	*
	* @param array	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);

		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1, 'value' => '');

		foreach ($this->_bodycopies as $name => $value) {
			$bodycopy = new Bodycopy();
			$bodycopy->setAttrValue('name', $name);
			$copy_link['value'] = $value;
			if (!$bodycopy->create($copy_link)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$this->id = 0;
				return false;
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	}//end create()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object Asset	$asset			the major or minor asset that we are linking to
	* @param string			$side_of_link	the side of the link we are on (major or minor)
	* @param string			$link_type		the type of link we are creating
	* @param string			$value			the value that is to be associated with the link
	* @param string			$sort_order		the position in the links list that this link should take,
	* @param string			$dependant		'0' / '1' on whether the this asset is dependant on the
	*										asset that will be linked by the new link
	* @param string			$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*										to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if this is a bodycopy then we need to make it a dependant link
		if ($side_of_link == 'major' && is_a($asset, 'bodycopy') && $dependant != '1') {
			$dependant = '1';
			return true;
		}

		return false;

	}//end prepareLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		// the only reason why we would object to this link being deleted is if it is a bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'major', null, '1', '1');

		if ($bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the bodycopy and the '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' page';

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


	/**
	* Returns an array of all the permitted link types, the asset type and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => count($this->_bodycopies), 'exclusive' => true);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$this->_current_state = '';
		$current_user = &$GLOBALS['SQ_SYSTEM']->user;
		set_error_handler(Array(&$this, '_errorHandler'));
			$success = $this->_processGlobalActions();
		restore_error_handler();

		// create a friendly error message on error
		if (isset($_POST['ASSET_BUILDER_ACTION']) && !$success) {
			$this->_replacements[strtolower($_POST['ASSET_BUILDER_ACTION']).'_error'] = $this->attr(strtolower($_POST['ASSET_BUILDER_ACTION']).'_error');

			if (!empty($this->_errors)) {
				$errors = '';
				foreach ($this->_errors as $text) $errors .= '<li>'.$text.'</li>';
				$this->_replacements[strtolower($_POST['ASSET_BUILDER_ACTION']).'_error'] .= '<ul>'.$errors.'</ul>';
			}
		}

		// if a new asset is created, it will be stored in the tmp vars for use by other asset types
		// that extend this one, but we wont need it any more - so chuck it out
		if (isset($this->_tmp['created_asset'])) unset($this->_tmp['created_asset']);

		if (empty($this->_current_state)) {
			if (is_null($current_user)) {
				// this person is not logged in, so we should let them log in
				$this->_current_state = 'not_logged_in';
				$this->_loadLoginReplacements();
			} else {
				$this->_current_state = 'logged_in';
			}
		}

		$fn = '_load'.$this->_default_state.'Replacements';
		$this->$fn();
		$this->_printBody();

	}//end printBody()


	/**
	* Prints the Asset Builder form and contents
	*
	* @return void
	* @access public
	*/
	function _printBody()
	{
		?>
		<script type="text/javascript" language="javascript" src="<?php echo sq_web_path('lib');?>/html_form/html_form.js"></script>
		<form name="asset_builder" method="POST" action="<?php echo $_SERVER['PHP_SELF']; ?>" enctype="multipart/form-data">
			<?php
			echo hidden_field('ASSET_BUILDER_ACTION', $this->_default_state);
			$this->_printContents();
			?>
		</form>
		<?php

	}//end _printBody()


	/**
	* Process any global Asset Builder actions
	*
	* These actions include logging into the system and creating an asset
	*
	* @return boolean	indicating success
	* @access private
	*/
	function _processGlobalActions()
	{
		$success = true;

		// process some global asset builder actions
		if (isset($_POST['ASSET_BUILDER_ACTION'])) {

			// okay, we are doing something - so success is not guaranteed
			$success = false;

			switch (strtolower($_POST['ASSET_BUILDER_ACTION'])) {

				case 'login' :
					// they have tried logging in - but failed
					// the error will get picked up down below though, so we dont have to do anything
					// uhhh... lazy days.... wish all coding was this easy
				break;

				case 'create' :

					// create our dummy asset so we can access all its you beaut functions
					// to make life easier for ourselves
					$create_type = $_POST['ASSET_BUILDER_CREATE_TYPE'];
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($create_type);
					$new_asset = new $create_type();
					$prefix = $new_asset->getPrefix();


					  ///////////////////////
					 //  REQUIRED FIELDS  //
					///////////////////////

					$required_error = false;
					$required_fields = $this->attr('required_fields');
					if (!empty($required_fields[$new_asset->type()])) {

						foreach ($required_fields[$new_asset->type()] as $attr => $error_text) {
							$real_attribute = &$new_asset->getAttribute($attr, true);
							if (is_null($real_attribute)) continue;
							// take a copy of the attribute so we don't muck anything up for the processBackend() below
							$attribute = $real_attribute;
							$attribute->process($prefix.'_'.$attribute->id);
							if (!$attribute->processed) {
								if (empty($error_text)) $error_text = 'The value of '.$attribute->name.' is not valid';
								trigger_error($error_text, E_USER_NOTICE);
								$required_error = true;
							} else if (empty($attribute->value)) {
								if (empty($error_text)) $error_text = 'Please enter a value for '.$attribute->name;
								trigger_error($error_text, E_USER_NOTICE);
								$required_error = true;
							}

						}//end foreach

						unset($real_attribute);
						unset($attribute);

					}// end if

					if ($required_error) break;

					  ////////////////////////
					 //  CREATE LOCATIONS  //
					////////////////////////

					if (($create_locations = $this->_getCreateLocations($create_type)) === FALSE) return false;

					// if we dont have any create locations, there is no point in continuing with this charade
					// whoever set this up may think they want to build asset on the frontend, but they dont really
					// know what they want - let them get back to us with something we can use
					if (empty($create_locations)) {
						trigger_error('Unable to determine where to create this '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($new_asset->type(), 'name'), E_USER_WARNING);
						break;
					}


					  ////////////////////////
					 //  CREATE THE ASSET  //
					////////////////////////

					// we'll just use a limbo outputter to do the outputting for us
					require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
					$o = new Limbo_Outputter();
					$create_parentid = array_shift(array_keys($create_locations));
					$link_type = array_shift(array_values($create_locations));
					$parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($create_parentid);
					if (is_null($parent)) {
						$this->_replacements['create_error'] = $this->attr('create_error');
						break;
					}

					// start the transaction to create the asset
					$link = Array('asset'		=> &$parent,
								  'link_type'	=> $create_locations[$create_parentid],
								  'value'		=> '',
								  'sort_order'	=> -1,
								  );

					$this->_modifyCreateLink($link);

					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

					// set the install flag because we are creating an asset without being
					// logged in - so any write access checks will fail
					$GLOBALS['SQ_INSTALL'] = true;

					if (!$new_asset->processBackend($o, $link)) {

						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');

					} else {

						// if we have multiple locations to create this asset under, create
						// the new links to all the other parents
						if (is_array($create_locations)) {
							foreach ($create_locations as $link_parentid => $link_type) {
								$link_parentid = (int)$link_parentid;

								// skip the orginal parent we linked the new asset to
								if ($link_parentid == $parent->id) continue;

								$link_parent = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link_parentid, '', true);
								if (is_null($link_parent)) {
									trigger_error('Asset $link_parentid was null', E_USER_WARNING);
									continue;
								}
								if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($link_parent, $new_asset, $link_type)) {
									trigger_error('Failed to create link to asset $link_parent->id', E_USER_WARNING);
									continue;
								}
							}
						}

						if (empty($this->_errors)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
							$this->_current_state = 'created';
							$success = true;
							$this->_setCreateStatus($new_asset);
							$this->_tmp['created_asset'] = $new_asset;
						}

					}//end if processBackend

					$GLOBALS['SQ_INSTALL'] = false;

				break;

				default :

					// there was an action, but we didnt do anything
					$success = true;

				break;

			}//end switch

		}//end if ASSET_BUILDER_ACTION

		return $success;

	}//end _processGlobalActions()


	/**
	* Work out the create locations for a new asset
	*
	* @param string	$create_type	the type of asset we are getting create locations for
	* @param array	$attributes		an array of attributes to use for rule comparison
	*								leave empty to use the submitted form vars
	*
	* @return array					parentid => link_type
	* @access private
	*/
	function _getCreateLocations($create_type, $attributes=Array())
	{

		if (empty($attributes)) {
			$create_type = $_POST['ASSET_BUILDER_CREATE_TYPE'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($create_type);
			$temp_asset = new $create_type();
			$prefix = $temp_asset->getPrefix();
			unset($temp_asset);
		}

		// start with the fixed create locations
		$create_locations = $this->attr('create_location');

		// add selected create locations if we are using the submitted form vars
		if (empty($attributes)) {
			foreach ($this->attr('create_location_optional') as $name => $details) {
				$selected_locations = Array();
				$code_name = str_replace(' ', '_', strtolower($name));
				if (isset($_POST[$code_name])) {
					if ($details['allow_multiple'] && is_array($_POST[$code_name])) {
						foreach ($_POST[$code_name] as $location_id) {
							// add it to the list
							$selected_locations[] = $location_id;
						}
						// check the list options are valid
						$selected_locations = array_intersect($selected_locations, array_keys($details['locations']));
						// add them to the collection
						foreach ($selected_locations as $location_id) $create_locations[(int)$location_id] = (int)$details['link_type'];
					} else {
						// if it's valid, add it to the list
						if (in_array($_POST[$code_name], array_keys($details['locations']))) {
							$create_locations[(int)$_POST[$code_name]] = (int)$details['link_type'];
						}
					}
				} elseif (!$details['allow_empty']) {
					trigger_error("You must select an option from the $name field", E_USER_ERROR);
					return false;
				}
			}
		}

		// now check what rules dictate where the asset should be created
		$location_rules = $this->attr('create_location_rules');
		if (!empty($location_rules[$create_type])) {
			foreach ($location_rules[$create_type] as $i => $rule) {
				$pass = true;
				$j = 0;
				while ($j < count($rule['attributes'])) {
					if (empty($attributes)) {
						$attr_value = isset($_POST[$prefix.'_'.$rule['attributes'][$j]]) ? trim(strtolower($_POST[$prefix.'_'.$rule['attributes'][$j]])) : '';
					} else {
						$attr_value = isset($attributes[$rule['attributes'][$j]]) ? trim(strtolower($attributes[$rule['attributes'][$j]])) : '';
					}

					switch ($rule['comparators'][$j]) {
						case 'equals':
							if ($attr_value != $rule['values'][$j]) $pass = false;
						break;

						case 'starts_with':
							if (!preg_match('|^'.$rule['values'][$j].'.*$|', $attr_value)) $pass = false;
						break;

						case 'ends_with':
							if (!preg_match('|^.*'.$rule['values'][$j].'$|', $attr_value)) $pass = false;
						break;

						case 'contains':
							if (strpos($attr_value, $rule['values'][$j]) === FALSE) $pass = false;
						break;
					}
					$j++;
				}

				if ($pass) {
					foreach ($rule['locations'] as $index => $assetid) {
						$create_locations[$assetid] = $rule['link_types'][$index];
					}
				}
			}
		}
		return $create_locations;

	}//end _getCreateLocations()


	/**
	* Takes the default link that was created and allows for adjustments if necessary
	*
	* @param Array $link	the current link array
	*
	* @return void
	* @access private
	*/
	function _modifyCreateLink(&$link)
	{
	}//end _modifyCreateLink()


	/**
	* Set the status of the newly created asset
	*
	* @param object Asset	&$new_asset		the newly created asset
	*
	* @return boolean
	* @access private
	*/
	function _setCreateStatus(&$new_asset)
	{
		$create_status = $this->attr('create_status');
		if ($new_asset->status != $create_status) {
			$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
			$vars = Array('assetid' => $new_asset->id, 'new_status' => $create_status);
			$status_errors = $hh->freestyleHipo('hipo_job_edit_status', $vars);
			$this->_errors = array_merge($this->_errors, $status_errors);
			return empty($this->_errors);
		}
		return true;

	}//end _setCreateStatus()


	/**
	* Loads keyword replacements for the logging in interface, into the Asset Builder
	*
	* @return void
	* @access private
	*/
	function _loadLoginReplacements()
	{
		$this->_replacements['login_invite'] = $this->attr('login_invite');

		ob_start();
			?>
			<input type="hidden" name="SQ_LOGIN_KEY"      value="<?php echo $GLOBALS['SQ_SYSTEM']->generateLoginKey(); ?>">
			<input type="hidden" name="SQ_LOGIN_REFERER"  value="">

			<table border="0">
				<tr>
					<td valign="top">Username</td>
					<td valign="top"><?php echo text_box('SQ_LOGIN_USERNAME', (isset($_POST['SQ_LOGIN_USERNAME'])) ? $_POST['SQ_LOGIN_USERNAME'] : 'Enter your username here', 25, 0, (isset($_POST['SQ_LOGIN_USERNAME'])) ? false : true, 'title="Username"'); ?></td>
				</tr>
				<tr>
					<td valign="top">Password</td>
					<td valign="top"><?php echo password_box('SQ_LOGIN_PASSWORD', '', 25, 0, 'title="Password"'); ?></td>
				</tr>
				<tr>
					<td valign="top">&nbsp;</td>
					<td valign="top"><?php echo submit_button('ASSET_BUILDER_ACTION', 'Login', 'this.form.action = "'.$_SERVER['PHP_SELF'].'?SQ_ACTION=login"; set_hidden_field("ASSET_BUILDER_ACTION", "login", document.asset_builder);'); ?></td>
				</tr>
			</table>

			<?php
			$this->_replacements['login_form'] = ob_get_contents();
		ob_end_clean();

	}//end _loadLoginReplacements()


	/**
	* Loads keyword replacements for the create asset interface, into the Asset Builder
	*
	* @return void
	* @access private
	*/
	function _loadCreateReplacements()
	{
		ob_start();
			$this->_replacements['create_invite'] = $this->attr('create_invite');

			// close you eyes while you go through this next bit of code
			// its not pretty and tries to fudge some stuff so we can pretend
			// we are in the backend when we are not even in limbo

			require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
			$o = new Limbo_Outputter();

			// print the create interface for the asset
			?><input type="hidden" name="asset_action" value="create"><?php
			$create_types = $this->attr('create_type');
			if (!is_array($create_types)) $create_types = Array($create_types);

			$i = 0;
			foreach($create_types as $create_type => $value) {
				$prefix = $this->type().'_'.$this->id.'_type_'.$create_type;
				?><div id="<?php echo $prefix ?>" style="display:<?php echo ($i==0) ? 'block' : 'none' ?>">
				<?php
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($create_type);
				$asset = new $create_type();
				$asset->paintBackend($o);
				// paint the buffered sections, then clean it up
				$o->paintSections();
				$o->_contents = Array();

				?></div><?php
				$i++;
			}

			$create_type = array_shift(array_keys($create_types));
			hidden_field('ASSET_BUILDER_CREATE_TYPE', $create_type);

			// okay, open your eyes again
			hidden_field('asset_action', 'create');
			$contents = ob_get_contents();
			$this->_replacements['create_form'] = $contents;

		ob_end_clean();

		if (count($create_types) > 1) {
			ob_start();
				$create_types = array_keys($create_types);
				?>
				<script language="JavaScript">
					<!--
					var current_option = '<?php echo $this->type().'_'.$this->id.'_type_'.current($create_types) ?>';

					function showAssetBuilderTab(type_code)
					{
						document.getElementById(current_option).style.display = 'none';
						document.getElementById(current_option+"_tab").className = 'sq-asset-builder-tab';
						current_option = '<?php echo $this->type().'_'.$this->id.'_type_' ?>' + type_code;
						document.getElementById(current_option).style.display = 'block';
						document.getElementById(current_option+"_tab").className = 'sq-asset-builder-tab-active';
						document.getElementById('ASSET_BUILDER_CREATE_TYPE').value = type_code;

					}//end showAssetBuilderTab()
					// -->
					</script>
				<?php
				$i = 0;
				?>
					<div class="sq-asset-builder-header">
				<?php
				foreach($create_types as $create_type) {
					$prefix = $this->type().'_'.$this->id.'_type_'.$create_type;
					?>
						<div class="sq-asset-builder-tab<?php echo ($i == 0) ? '-active' : ''; ?>" id="<?php echo $prefix.'_tab'; ?>">
							<a href="#" onclick="showAssetBuilderTab('<?php echo $create_type ?>'); return false;">
								<?php
								$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($create_type, 'name');
								echo $type_name;
								?>
							</a>
						</div>
					<?php
					$i++;
				}
				?>
					</div>
				<?php
				$this->_replacements['create_form'] = ob_get_contents().$this->_replacements['create_form'];
			ob_end_clean();
		}

		foreach ($this->attr('create_location_optional') as $name => $data) {
			$code_name = str_replace(' ', '_', strtolower($name));
			$options = $data['locations'];

			ob_start();
			switch ($data['input_style']) {

				case 'dropdown':
					if ($data['allow_empty'] && !$data['allow_multiple']) {
						$options = Array(''=>'') + $options;
					}
					$default = $data['allow_empty'] ? Array() : key($options);
					$height = $data['allow_multiple'] ? 4 : 1;
					combo_box($code_name, $options, $data['allow_multiple'], $default,  $height);
				break;

				case 'radio':
					if ($data['allow_multiple']) {
						// print checkboxes
						$checked = !$data['allow_empty'];
						$i = 0;
						foreach ($data['locations'] as $id => $name) {
							check_box($code_name."[$i]", $id, $checked);
							echo '&nbsp;' . $name . '<br />';
							$checked = false;
							$i++;
						}
					} else {
						$checked = !$data['allow_empty'];
						foreach ($data['locations'] as $id => $name) {
							radio_button($code_name, $id, $checked);
							echo '&nbsp;' . $name . '<br />';
							$checked = false;
						}
					}
				break;

				default:
					echo '(UNDEFINED INPUT STYLE ' . $data['input_style'] . ')';
				break;
			}
			$this->_replacements[$code_name.'_field'] = ob_get_contents();
			ob_end_clean();

		}

	}//end _loadCreateReplacements()


	/**
	* Print the contents of the Asset Builder page with replaced keywords
	*
	* @return boolean
	* @access public
	*/
	function _printContents()
	{
		// get the bodycopy attached to us
		$bodycopy_data = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', true, $this->_current_state, 'major', '1');
		if (empty($bodycopy_data)) return;
		$bodycopy = &$GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_data['minorid'], $bodycopy_data['minor_type_code']);
		if (is_null($bodycopy)) return;

		$bodycopy->setKeywordReplacements($this->_replacements);

		$bodycopy->printBody();

	}//end _printContents()


	/**
	* This function captures all errors and warnings that occur to hide them from the user
	*
	* @param int	$err_no		The type of error (E_*)
	* @param string	$err_msg	The error message
	* @param string	$err_file	The file the error occured in
	* @param string	$err_line	The line the error occured on
	*
	* @returns void
	* @access private
	* @see HIPO_Job::process()
	*/
	function _errorHandler($err_no, $err_msg, $err_file, $err_line)
	{
		$terminate = ((E_USER_ERROR | E_ERROR) & $err_no);

		// if the function didn't have an '@' prepended OR if we are about to terminate
		// catch the error
		if (error_reporting() || $terminate) {

			// Strip out the file path begining
			$err_file = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), $err_file);
			$err_msg  = str_replace(Array("\r", SQ_SYSTEM_ROOT), Array('', '[SYSTEM_ROOT]'), $err_msg);

			$text_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));

			// send a report to the system error log
			if (ini_get('log_errors')) {
				$err_name = '';
				switch($err_no) {
					case E_USER_ERROR	: $err_name = SQ_SYSTEM_SHORT_NAME.' Error';	break;
					case E_USER_WARNING	: $err_name = SQ_SYSTEM_SHORT_NAME.' Warning';	break;
					case E_USER_NOTICE	: $err_name = SQ_SYSTEM_SHORT_NAME.' Notice';	break;
					case E_ERROR		: $err_name = 'PHP Error';						break;
					case E_WARNING		: $err_name = 'PHP Warning';					break;
					case E_NOTICE		: $err_name = 'PHP Notice';						break;
					default				: $err_name = 'Unknown Error Type - '.$err_no;
				}

				$log_msg = date('D d/m/Y H:i:s').' ['.strtolower($err_name).'] ('.$err_file.':'.$err_line.') - '.$text_msg;
				$log_msg = str_replace('[SYSTEM_ROOT]', '', $log_msg);
				error_log($log_msg);
			}

			$this->_errors[] = $text_msg;

		}// end error_reporting

		if ($terminate) exit(1);

	}//end _errorHandler()


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param &object Asset	$asset	the asset that triggered the event
	* @param Array			$vars	the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) return;

		$keywords = Array();
		foreach ($this->_available_keywords as $keyword) {
			$name = ucwords(str_replace('_', ' ', $keyword));
			$type = 'inline';
			if ($keyword == 'create_location') {
			} else if (substr($keyword, -5) == '_form') {
				$default = $name;
				$type = 'block';
			} else {
				$default = $this->attr($keyword);
			}
			$keywords[$keyword] = Array('name' => $name, 'default' => $default, 'type' => $type);
		}
		$dropdown_default = '<select><option>%name% Option 1</option><option>%name% Option 2</option></select>';
		$radio_default = '<input type="radio" />%name% Option 1 <br /><input type="radio" />%name% Option 2 <br />';

		foreach ($this->attr('create_location_optional') as $name => $data) {
			$kw_name = str_replace(' ', '_', strtolower($name)).'_field';
			$keywords[$kw_name] = Array('name' => $name.' Field',
										'default' => str_replace('%name%', $name, (($data['input_style'] == 'dropdown') ? $dropdown_default : $radio_default)),
										'type' => 'special');
		}
		$vars['keywords'] = array_merge($vars['keywords'], $keywords);

	}//end onRequestKeywords()


}//end class

?>
