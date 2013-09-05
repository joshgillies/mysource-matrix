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
* $Id: page_asset_builder.inc,v 1.182 2013/09/05 03:13:53 akarelia Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';
require_once SQ_FUDGE_PATH.'/mollom/mollom.inc';
require_once SQ_FUDGE_PATH.'/recaptcha/recaptcha.inc';


/**
* Page_Asset_Builder
*
* Build Assets from the frontend
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @author  Avi Miller <avi.miller@squiz.net>
* @author  Marcus Nyeholt <mnyeholt@squiz.net>
* @version $Revision: 1.182 $
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
	* @var array
	*/
	var $_errors = Array();

	/**
	* An array of keyword replacements to make
	* @var array
	*/
	var $_replacements = Array();

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
	function __construct($assetid=0)
	{
		$this->_ser_attrs = TRUE;
		parent::__construct($assetid);

	}//end constructor


	/**
	* Perform any additional processing required during the creation of this asset
	*
	* Asset builders create bodycopy assets when they are created
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _createAdditional(&$link)
	{
		if (!parent::_createAdditional($link)) return FALSE;

		// add a bodycopy to this page when creating
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'is_dependant' => 1, 'is_exclusive' => 1, 'value' => '');

		foreach ($this->_getBodycopies() as $value => $details) {
			$bodycopy = new Bodycopy();
			$bodycopy->setAttrValue('name', $details['name']);
			$copy_link['value'] = $value;
			unset($details['name']);
			if (!$bodycopy->create($copy_link, $details)) {
				return FALSE;
			}
		}
		return TRUE;

	}//end _createAdditional()


	/**
	* Save attributes set with setAttrValue to the database
	*
	* @param boolean	$dont_run_updated	if TRUE this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see setAttrValue()
	*/
	function saveAttributes($dont_run_updated=FALSE)
	{
		if (!parent::saveAttributes($dont_run_updated)) {
			return FALSE;
		}

		// Create / remove layouts
		// Bug Fix : #3360 Cloning asset builder with customised create layout fail
		// We now check is Asset Exists, because while cloning it is trying to link the layout
		// to the page_asset_builder Asset when it doesnt exist on the system
		if ($this->id && !$GLOBALS['SQ_SYSTEM']->am->assetInTrash($this->id, TRUE)) {
			$custom_types = $this->attr('create_form_customisations');
			$current_layout_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'layout', TRUE);
			$current_layouts = Array();
			foreach ($current_layout_links as $link) {
				if (strpos($link['value'], '_logged_in_customisation') !== FALSE) continue;
				if (in_array($link['value'], $custom_types)) {
					$current_layouts[] = $link['value'];
				} else {
					$GLOBALS['SQ_SYSTEM']->am->deleteAssetLink($link['linkid']);
				}
			}
			$types_needing_layouts = array_diff($custom_types, $current_layouts);
			if (!empty($types_needing_layouts)) {
				$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($types_needing_layouts, 'name');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('layout');
				foreach ($types_needing_layouts as $type_code) {
					$layout = new Layout();
					$layout->setAttrValue('name', translate('cms_asset_builder_create_form_layout_for_type', $type_names[$type_code]));
					$link = Array(
								'asset'			=> &$this,
								'link_type'		=> SQ_LINK_TYPE_2,
								'is_dependant'	=> TRUE,
								'is_exclusive'	=> TRUE,
								'value'			=> $type_code,
							);
					$layout->create($link);
				}
			}
		}
		return TRUE;

	}//end saveAttributes()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param object	&$asset			the major or minor asset that we are linking to
	* @param string	$side_of_link	the side of the link we are on (major or minor)
	* @param string	&$link_type		the type of link we are creating
	* @param string	&$value			the value that is to be associated with the link
	* @param string	&$sort_order	the position in the links list that this link should take,
	* @param string	&$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	function prepareLink(&$asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if this is a bodycopy then we need to make it a dependant link
		if ($side_of_link == 'major' && ($asset instanceof Bodycopy) && $dependant != '1') {
			$dependant = '1';
			return TRUE;
		}

		return FALSE;

	}//end prepareLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed boolean|string
	* @access public
	*/
	function isDeletableLink($linkid)
	{
		// the only reason why we would object to this link being deleted is if it is a bodycopy linked to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'major', NULL, '1', '1');

		if ($bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) {
			return translate('cms_cannot_delete_link', $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'));
		}

		return parent::isDeletableLink($linkid);

	}//end isDeletableLink()


	/**
	* Returns an array of all the permitted link types, the asset type and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_TYPE_2]['bodycopy'] = Array('card' => 'M', 'exclusive' => TRUE);
		$page_links[SQ_LINK_NOTICE]['metadata_schema'] = Array('card' => 'M', 'exclusive' => FALSE);
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
		if (isset($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION'])) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		}

		$this->_current_state = '';
		$current_user = $GLOBALS['SQ_SYSTEM']->user;
		set_error_handler(Array(&$this, '_errorHandler'));
			$success = $this->_processGlobalActions();
		restore_error_handler();

		// create a friendly error message on error
		if (isset($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION']) && !$success) {
			$this->_replacements[strtolower($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION']).'_error'] = $this->attr(strtolower($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION']).'_error');

			if (!empty($this->_errors)) {
				$errors = '';
				foreach ($this->_errors as $text) {
					$errors .= '<li>'.$text.'</li>';
				}
				$this->_replacements[strtolower($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION']).'_error'] .= '<ul>'.$errors.'</ul>';
			}
		}

		// start performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'printBody');

		// if a new asset is created, it will be stored in the tmp vars for use by other asset types
		// that extend this one, but we wont need it any more - so chuck it out, but first, load up
		// some replacements for the newly created asset
		if (isset($this->_tmp['created_asset'])) {
			$this->_loadAssetReplacements();
			unset($this->_tmp['created_asset']);
		}

		if (empty($this->_current_state)) {
			if (is_null($current_user) || ($current_user instanceof Public_User)) {
				// this person is not logged in, so we should let them log in
				$this->_current_state = 'not_logged_in';
				$this->_replacements['login_invite'] = $this->getKeywordReplacement('login_invite');
				$this->_replacements['login_form'] = $this->getKeywordReplacement('login_form');
			} else {
				$this->_current_state = 'logged_in';
			}
		}

		$fn = '_load'.$this->_default_state.'Replacements';
		$this->$fn();
		$this->_printBody();

		// stop performance mode timer
		$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'printBody');

		if (isset($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION'])) {
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
		}		

	}//end printBody()


	/**
	* Prints the Asset Builder form and contents
	*
	* @return void
	* @access public
	*/
	function _printBody()
	{
		if ($this->_current_state == 'created'){
			$this->_printContents();
		} else {
			$submission_url = $this->attr('submit_to_page_url') ? replace_query_string_vars(Array(), $this->getURL()) : replace_query_string_vars(Array());

			// Mark the form load time
			if (!isset($_SESSION['SQ_AB_'.$this->id.'_LOAD_TIME'])) {
				$_SESSION['SQ_AB_'.$this->id.'_LOAD_TIME'] = time();
			}

			?>
			<form id="<?php echo $this->getPrefix() ?>" method="post" action="<?php echo htmlentities($submission_url); ?>" enctype="multipart/form-data" onsubmit="return form_on_submit()">
				<?php
				hidden_field('AB_'.$this->id.'_ASSET_BUILDER_ACTION', $this->_default_state);
				$parameter_map = $this->getAttribute('parameter_map');
				if (($additional_locs = $parameter_map->getParameterValue('additional_locations')) != NULL) {
					hidden_field('AB_'.$this->id.'_ASSET_BUILDER_ADDITIONAL_LOCATIONS', $additional_locs);
				} else if (isset($_POST['AB_'.$this->id.'_ASSET_BUILDER_ADDITIONAL_LOCATIONS'])) {
					hidden_field('AB_'.$this->id.'_ASSET_BUILDER_ADDITIONAL_LOCATIONS', $_POST['AB_'.$this->id.'_ASSET_BUILDER_ADDITIONAL_LOCATIONS']);
				}

				$this->_printContents();
				?>
			</form>
			<?php
		}

	}//end _printBody()


	/**
	* Form Captcha keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaKeywordReplacement()
	{
		if (!$this->attr('captcha_ttf') == '') {
			$font = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('captcha_ttf'));
			$file_path = $font->data_path.'/'.$font->attr('name');
		} else {
			$file_path = '';
		}

		ob_start();
			security_key(
				$this->attr('captcha_length'),
				$this->attr('captcha_length'),
				$this->attr('captcha_zoom'),
				FALSE,
				trim($this->attr('captcha_bgcolour'), '#'),
				trim($this->attr('captcha_textcolour'), '#'),
				trim($this->attr('captcha_bordercolour'), '#'),
				$this->attr('captcha_use_colours'),
				$this->attr('captcha_usefont'),
				$file_path,
				$this->attr('captcha_font_size'),
				$this->attr('captcha_min_angle'),
				$this->attr('captcha_max_angle'),
				$this->attr('captcha_x_start'),
				$this->attr('captcha_min_distance'),
				$this->attr('captcha_max_distance'),
				$this->attr('captcha_width'),
				$this->attr('captcha_height'),
				$this->attr('captcha_arc'),
				trim($this->attr('captcha_arc_colour'), '#'),
				$this->attr('captcha_include_uppercase'),
				$this->attr('captcha_include_numbers'),
				$this->attr('provide_accessible_captcha'),
				FALSE,
				$this->attr('captcha_label_text')
			);
		return ob_get_clean();

	}//end getFormCaptchaKeywordReplacement()


	/**
	* Form Captcha Image keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaImageKeywordReplacement()
	{
		if (!$this->attr('captcha_ttf') == '') {
			$font = $GLOBALS['SQ_SYSTEM']->am->getAsset($this->attr('captcha_ttf'));
			$file_path = $font->data_path.'/'.$font->attr('name');
		} else {
			$file_path = '';
		}

		ob_start();
			security_key_image_box(
				$this->attr('captcha_length'),
				$this->attr('captcha_zoom'),
				trim($this->attr('captcha_bgcolour'), '#'),
				trim($this->attr('captcha_textcolour'), '#'),
				trim($this->attr('captcha_bordercolour'), '#'),
				$this->attr('captcha_use_colours'),
				$this->attr('captcha_usefont'),
				$file_path,
				$this->attr('captcha_font_size'),
				$this->attr('captcha_min_angle'),
				$this->attr('captcha_max_angle'),
				$this->attr('captcha_x_start'),
				$this->attr('captcha_min_distance'),
				$this->attr('captcha_max_distance'),
				$this->attr('captcha_width'),
				$this->attr('captcha_height'),
				$this->attr('captcha_arc'),
				trim($this->attr('captcha_arc_colour'), '#'),
				$this->attr('captcha_include_uppercase'),
				$this->attr('captcha_include_numbers'),
				$this->attr('provide_accessible_captcha')
			);
		return ob_get_clean();

	}//end getFormCaptchaImageKeywordReplacement()


	/**
	* Form Captcha Field keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaFieldKeywordReplacement()
	{
		ob_start();
			security_key_field($this->attr('captcha_length'));
		return ob_get_clean();

	}//end getFormCaptchaFieldKeywordReplacement()


	/**
	* Form Captcha Link keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaLinkKeywordReplacement()
	{
		ob_start();
			security_key_image_link($this->attr('captcha_link_text'));
		return ob_get_clean();

	}//end getFormCaptchaLinkKeywordReplacement()


	/**
	* Form Captcha Field keyword replacement
	*
	* @return string
	* @access public
	*/
	function getFormCaptchaLabelKeywordReplacement()
	{
		return $this->attr('captcha_label_text');

	}//end getFormCaptchaLabelTextKeywordReplacement()


	/**
	* Process any global Asset Builder actions
	*
	* These actions include logging into the system and creating an asset
	*
	* @return boolean
	* @access private
	*/
	function _processGlobalActions()
	{
		$success = TRUE;

		// process some global asset builder actions
		if (isset($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION'])) {

			// okay, we are doing something - so success is not guaranteed
			$success = FALSE;

			switch (strtolower($_POST['AB_'.$this->id.'_ASSET_BUILDER_ACTION'])) {

				case 'login' :
					// they have tried logging in - but failed
					// the error will get picked up down below though, so we dont have to do anything
					// uhhh... lazy days.... wish all coding was this easy
				break;

				case 'create' :

					$required_error = FALSE;

					// verify captcha
					if ($this->attr('require_captcha')) {
						if (!validate_security_key()) {
							$error_text = $this->attr('captcha_invalid_error_message');
							trigger_error($error_text, E_USER_NOTICE);
							$required_error = TRUE;
						}
					}

					// create our dummy asset so we can access all its you beaut functions
					// to make life easier for ourselves
					$create_type = $_POST['AB_'.$this->id.'_ASSET_BUILDER_CREATE_TYPE'];
					$allowed_types = $this->attr('create_type');
					if(!isset($allowed_types[$create_type])) {
						trigger_error('Create type is not allowed', E_USER_NOTICE);
						return FALSE;
					}
					$GLOBALS['SQ_SYSTEM']->am->includeAsset($create_type);
					$new_asset = new $create_type();
					$prefix = $new_asset->getPrefix();


					  ///////////////////////
					 //  REQUIRED FIELDS  //
					///////////////////////
					// start performance mode timer
					$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'checkRequiredFields');

					$required_fields = $this->attr('required_fields');
					if (!empty($required_fields[$new_asset->type()])) {

						foreach ($required_fields[$new_asset->type()] as $attr => $error_text) {
							$pos = strpos($attr, $this->getPrefix().'_metadata_');
							if ($pos !== FALSE) {
								// Process metadata
								$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
								$am = $GLOBALS['SQ_SYSTEM']->am;
								$metadata_values = Array();
								$cascade_values_asset_ids = Array();
								$metadata_schemas = Array();
								$metadata_schema_list = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'schema_to_apply');
								foreach ($metadata_schema_list as $metadata_schema) {
									$metadata_schemas[] = $metadata_schema['minorid'];
								}
								if (!empty($metadata_schemas)) {
									$create_locations = $this->getCreateLocations($create_type);
									$metadata_field = substr($attr, strlen($this->getPrefix().'_metadata_'));
									$mf_asset = $am->getAsset($metadata_field);
									$valid_value = TRUE;
									$empty_value = TRUE;

									// Validate the metadata value
									if (!is_null($mf_asset)) {
										$edit_fns = $mf_asset->getEditFns();
										$cascade = TRUE;
										$did_process = $edit_fns->processInlineValueInterface($mf_asset, $result_value, $cascade, $create_type);

										// When date is not supplied for metadata date field
										if ($mf_asset->type() == 'metadata_field_date' && $result_value == '---------- --:--:--') {
											$result_value = '';
										}

										if (!$did_process) {
											$valid_value = FALSE;
										}//end if
										if (empty($result_value)) {
											// Maybe using default
											if (!is_null($result_value)) {
												$empty_value = FALSE;
											}
										}//end if
									}//end if


									if (!$valid_value) {
										if (empty($error_text)) {
											$error_text = translate('cms_asset_builder_invalid_attr_value', $mf_asset->name);
										}
										trigger_error($error_text, E_USER_NOTICE);
										$required_error = TRUE;
									} else if (!$empty_value) {
										if (empty($error_text)) {
											$error_text = translate('cms_asset_builder_missing_attr', $mf_asset->name);
										}
										trigger_error($error_text, E_USER_NOTICE);
										$required_error = TRUE;
									}//end else

									// Forget a few things
									$am->forgetAsset($schema);
									$am->forgetAsset($mf_asset);
									$am->forgetAsset($temp_asset);
									unset($temp_asset);

								}//end if
							} else {
								// Process attributes
								$real_attribute = $new_asset->getAttribute($attr, TRUE);
								if (is_null($real_attribute)) continue;
								// take a copy of the attribute so we don't muck anything up for the processBackend() below
								$attribute = clone $real_attribute;
								$attribute->process($prefix.'_'.$attribute->id);

								// For attributes that use paint and process functions, they return an empty result
								// So we should try again with a different param
								// See Bug #3099
								if (empty($attribute->value)) {
									$attribute->process($prefix.'_'.$attribute->name);
								}//end if

								if (!$attribute->processed) {
									if (empty($error_text)) {
										$error_text = translate('cms_asset_builder_invalid_attr_value', $attribute->name);
									}
									trigger_error($error_text, E_USER_NOTICE);
									$required_error = TRUE;
								} else if (empty($attribute->value)) {
									if (empty($error_text)) {
										$error_text = translate('cms_asset_builder_missing_attr', $attribute->name);
									}
									trigger_error($error_text, E_USER_NOTICE);
									$required_error = TRUE;
								}

								unset($real_attribute);
								unset($attribute);

							}//end else
						}//end foreach

					}//end if

					// stop performance mode timer
					$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'checkRequiredFields');
					if ($required_error) break;

					  ////////////////////////
					 //  CREATE LOCATIONS  //
					////////////////////////
					// start performance mode timer
					$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'getCreateLocations');
					if (($create_locations = $this->getCreateLocations($create_type)) === FALSE) {
						return FALSE;
					}
					// stop performance mode timer
					$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'getCreateLocations');

					// if we dont have any create locations, there is no point in continuing with this charade
					// whoever set this up may think they want to build asset on the frontend, but they dont really
					// know what they want - let them get back to us with something we can use
					if (empty($create_locations)) {
						trigger_localised_error('CMS0033', E_USER_WARNING, $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($new_asset->type(), 'name'));
						break;
					}


					  ////////////////////////
					 //  CREATE THE ASSET  //
					////////////////////////
					// start performance mode timer
					$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'createAsset');

					// we'll just use a limbo outputter to do the outputting for us
					require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
					$o = new Limbo_Outputter();
					$create_location_key = array_keys($create_locations);
					$create_parentid = $create_location_key[0];
					$create_location_value = array_values($create_locations);
					$link_type = $create_location_value[0];
					$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($create_parentid);
					if (is_null($parent)) {
						$this->_replacements['create_error'] = $this->getKeywordReplacement('create_error');
						break;
					}

					// If the link value is set, grab it, otherwise return a blank ''
					$grab_link_value = $this->attr('link_value');
					if (!empty($grab_link_value)) {
						$link_value = trim($grab_link_value);
					} else {
						$link_value = '';
					}

					// start the transaction to create the asset
					$link = Array(
								'asset'			=> &$parent,
								'link_type'		=> $create_locations[$create_parentid],
								'value'			=> $link_value,
								'sort_order'	=> -1,
							);

					$this->_modifyCreateLink($link);

					$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
					$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

					// set the install run level because we are creating an asset without being
					// logged in - so any write access checks will fail
					if ($this->attr('allow_public_user')) {
						$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					}

					// Fast track!
					// Due to the way that events are broadcast and handled, and the complexity introduced with
					// contexts, we can find ourselves re-regenerating/reindexing metadata, and firing off
					// multiple trigger events for metadata regeneration with the same details.
					//
					// Using these 'fast track' settings, we can skip work we know will be completed later.
					// They are unset accordingly later..
					$new_asset->fastTrack(	Array(
												'metadata_manager_generate_content_file',
												'search_manager_reindex_metadata',
												'trigger_event_metadata_updated',
											));

					$create_success = $new_asset->processBackend($o, $link);
					$new_asset_linkid = FALSE;
					if ($create_success && ($_REQUEST['asset_action'] != 'create')) {
						$new_asset_linkid = $new_asset->create($link);
						$create_success = $create_success && $new_asset_linkid;
					} else if ($create_success) {
						$new_asset_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($parent->id, $new_asset->id);
						$new_asset_linkid = array_get_index($new_asset_link, 'linkid', FALSE);
					}

					if (!$create_success) {
						$this->_tmp['new_asset_created_'.$create_type] =& $new_asset;
						$this->_abortAssetCreate($new_asset_linkid);
					} else {
						// Attributes to be verified by Mollom
						$attr_to_verify_assets = $this->attr('mollom_attributes_to_check');
						$attr_to_verify = isset($attr_to_verify_assets[$create_type]) ? $attr_to_verify_assets[$create_type] : Array();
						$mollom_parameters = Array();

						// make sure text attributes dont have javascripts etc
						foreach ($new_asset->vars as $attr_name => $attr_info) {
							// Prepare parameters for Mollom spam verification
							$attr_value = array_get_index($attr_info, 'value', '');
							if ($attr_value) {
								$attr_content_type = array_get_index($attr_to_verify, $attr_name, -1);
								if ($attr_content_type >= 0) {
									$mollom_parameters[$attr_content_type] = $attr_value;
								}
							}

							if ($attr_info['type'] == 'text') {
								if ($attr_name == 'name' || $attr_name == 'short_name') {
									$attr_name_value = htmlspecialchars($attr_info['value'], ENT_NOQUOTES);
									$attr_name_value = str_replace('&amp;', '&', $attr_name_value);
									$new_asset->setAttrValue($attr_name, $attr_name_value);
								} else {
									$attr_name_value = htmlspecialchars($attr_info['value']);
									$attr_name_value = str_replace('&amp;', '&', $attr_name_value);
									$new_asset->setAttrValue($attr_name, $attr_name_value);
								}
							}
						}


						// locks need to be acquired here in case any attribute values change above
						// saveAttributes() is smart enough to only run queries if values have changed since creation, i.e. a value gets escaped above
						$GLOBALS['SQ_SYSTEM']->am->acquireLock($new_asset->id, 'attributes');
						$new_asset->saveAttributes();
						$GLOBALS['SQ_SYSTEM']->am->releaseLock($new_asset->id, 'attributes');

						// stop performance mode timer
						$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'createAsset');

						// start performance mode timer
						$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'processMetadata');

						// process metadata
						$mm = $GLOBALS['SQ_SYSTEM']->getMetadataManager();
						$metadata_values = Array();
						$schemaids = $this->_getMetadataSchemasToApply();
						$cascade_values_asset_ids = Array();
						if (!empty($schemaids)) {
							$metadata_run_level_changed = FALSE;
							if ($GLOBALS['SQ_SYSTEM']->runLevelEnables(SQ_SECURITY_PERMISSIONS)) {
								$GLOBALS['SQ_SYSTEM']->setRunLevel($GLOBALS['SQ_SYSTEM']->getRunLevel() - SQ_SECURITY_PERMISSIONS);
								$metadata_run_level_changed = TRUE;
							}
							$GLOBALS['SQ_SYSTEM']->am->acquireLock($new_asset->id, 'metadata');
							foreach ($schemaids as $schemaid) {
								$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
								$mm->setSchema($new_asset->id, $schema->id, TRUE);
								if (!is_null($schema)) {
									$edit_fns = $schema->getEditFns();
									$edit_fns->processInlineValueInterface($schema, $metadata_values, $cascade_values_asset_ids, $create_type);
								}
							}

							// set metadata without reindexing and reset the switch to skip reindex
							$mm->setMetadata($new_asset->id, $metadata_values);

							// Fast track's mission is accomplished, reset.
							$new_asset->unFastTrack(	Array(
														'metadata_manager_generate_content_file',
														'search_manager_reindex_metadata',
														'trigger_event_metadata_updated',
													));

							$mm->regenerateMetadata($new_asset->id, 'all');

							$GLOBALS['SQ_SYSTEM']->am->releaseLock($new_asset->id, 'metadata');
							if ($metadata_run_level_changed) {
								$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
							}

							// Set metadata values for Mollom check, if any
							foreach($metadata_values as $fieldid => $m_data) {
								$attr_content_type = array_get_index($attr_to_verify, $this->getPrefix().'_metadata_'.$fieldid, -1);
								if ($attr_content_type >= 0) {
									$m_value = array_get_index($m_data[0], 'value', '');
									if (!empty($m_value)) {
										$mollom_parameters[$attr_content_type] = $m_value;
									}
								}
							}//end foreach
						}

						// stop performance mode timer
						$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'processMetadata');

						// Carryout Mollom spam verification
						if ($this->attr('use_mollom') && !empty($mollom_parameters)) {
							$mollom_result = Mollom::verifySubmission($this->id, $mollom_parameters);
							switch ($mollom_result) {
								case SQ_MOLLOM_SPAM:
									// Mollom has identified this submission as spam
									$error_text = $this->attr('mollom_spam_submission_text');
									if ($error_text) {
										$this->_errors[] = $error_text;
									} else {
										$this->_errors[] = translate('cms_asset_builder_mollom_spam_submission_message');
									}
								break;

								case SQ_MOLLOM_UNSURE:
									// Mollom is not sure about the submission
									$error_text = $this->attr('mollom_unsure_submission_text');
									if ($error_text) {
										$this->_errors[] = $error_text;
									} else {
										$this->_errors[] = translate('cms_asset_builder_mollom_unsure_spam_submission_message');
									}
								break;

								case SQ_MOLLOM_HAM:
									// Everything cool
								break;

								case SQ_MOLLOM_UNKNOWN:
									// No response from Mollom
								break;

								case SQ_MOLLOM_INCORRECT_CAPTCHA:
									$error_text = $this->attr('mollom_captcha_incorrect_text');
									if($error_text) {
										$this->_errors[] = $error_text;
									} else {
										$this->_errors[] = translate('cms_asset_builder_mollom_invalid_captcha_message');
									}
								break;
							}//end switch
						}//end if use mollom

						// reCAPTCHA check
						if ($this->attr('use_recaptcha')) {
							$recaptcha_response = Recaptcha::verifyRecaptcha();
							if (!$recaptcha_response->is_valid) {
								$error_msg = $this->attr('recaptcha_error_message');
								if (empty($error_msg)) {
									$error_msg = array_shift($recaptcha_response->error);
								}
								$this->_errors[] = $error_msg;
							}
						}//end if

						// Honeypot question validation
						if (!$this->_honeypotFieldEmpty()) {
							$this->_errors[] = $this->attr('honeypot_error_message');
						}//end if

						// Submission time requirement check
						if (!$this->_passedMinimumTimeThreshold()) {
							$this->_errors[] = $this->attr('time_requirement_error_message');
						}//end if

						// start performance mode timer
						$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'processLayouts');

						// process paint layouts
						$layouts['paint_layout::system::frontend'] = $this->attr('layout_to_apply');
						$layouts['override::paint_layout::system::frontend'] = $this->attr('override_layout_to_apply');
						$new_asset->setPaintLayouts($layouts);

						// stop performance mode timer
						$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'processLayouts');

						// start performance mode timer
						$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'linkMultipleLocations');

						// if we have multiple locations to create this asset under, create
						// the new links to all the other parents
						if (is_array($create_locations)) {
							foreach ($create_locations as $link_parentid => $link_type) {

								// skip the orginal parent we linked the new asset to
								if ($link_parentid == $parent->id) continue;

								$link_parent = $GLOBALS['SQ_SYSTEM']->am->getAsset($link_parentid, '', TRUE);
								if (is_null($link_parent)) {
									trigger_localised_error('CMS0034', E_USER_WARNING, $link_parentid);
									continue;
								}
								if (!$GLOBALS['SQ_SYSTEM']->am->createAssetLink($link_parent, $new_asset, $link_type, $link_value)) {
									trigger_localised_error('CMS0035', E_USER_WARNING, $link_parent->id);
									continue;
								}

								$current_paths = $new_asset->getWebPaths();

								// ensure we always have valid web paths if more than one create location
								require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
								$valid_paths = make_valid_web_paths($current_paths, $new_asset->id);
								$valid_paths = $GLOBALS['SQ_SYSTEM']->am->webPathsInUse($link_parent, $valid_paths, $new_asset->id, TRUE);

								if (!$new_asset->saveWebPaths($valid_paths, FALSE)) {
									$this->_errors[] = translate('cms_asset_builder_cannot_save_web_path', $new_asset->name, $link_parent->name);
								}
								// if there is no difference in the arrays
								// we have to update lookups mannualy, otherwise it would have be done in saveWebPath already
								if (!array_diff_assoc($valid_paths, $current_paths) && !array_diff_assoc($current_paths, $valid_paths)) {
									$new_asset->updateLookups();
								}
							}
						}

						// stop performance mode timer
						$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'linkMultipleLocations');

						// start performance mode timer
						$GLOBALS['SQ_SYSTEM']->pm->startTimer($this, 'setCreateStatus');

						if (empty($this->_errors)) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							$this->_current_state = 'created';
							$success = TRUE;
							$this->_setCreateStatus($new_asset);
							$this->_tmp['created_asset'] = $new_asset;

							if (isset($_SESSION['SQ_AB_'.$this->id.'_LOAD_TIME'])) {
								unset($_SESSION['SQ_AB_'.$this->id.'_LOAD_TIME']);
							}

							// if we have reached so far the asset has been built, even with regards to Account Manager Asset,
							// user is created already, so lets celebrate firing (trigger) !!!
							$GLOBALS['SQ_SYSTEM']->broadcastTriggerEvent('trigger_event_after_asset_built', $new_asset);
						} else {
							$this->_abortAssetCreate($new_asset_linkid);
						}

						// stop performance mode timer
						$GLOBALS['SQ_SYSTEM']->pm->stopTimer($this, 'setCreateStatus');
					}//end else

					if ($this->attr('allow_public_user')) {
						$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
					}

				break;

				default :
					// there was an action, but we didnt do anything
					$success = TRUE;

				break;

			}//end switch

		}//end if ASSET_BUILDER_ACTION

		return $success;

	}//end _processGlobalActions()


	/*
	* Abort the asset creation (asset created by Asset Builder)
	*
	* @param int $linkid 	Linkid of the asset that failed while getting created
	*
	* @return void
	* @access private
	*/
	function _abortAssetCreate($linkid=0)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		// Clean up the redundent links left in the link table
		if ($linkid) {
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db3');

			$sql = '
				SELECT t.treeid
				FROM sq_ast_lnk_tree t INNER JOIN sq_ast_lnk l ON t.linkid = l.linkid
				WHERE l.linkid = :linkid';
			$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(), 1);

            try {
                $query = MatrixDAL::preparePdoQuery($sql);
                MatrixDAL::bindValueToPdo($query, 'linkid', $linkid);
                $treeid = MatrixDAL::executePdoOne($query);
            } catch (Exception $e) {
                throw new Exception('Unable to get treeid for linkid: '.$linkid.' due to database error: '.$e->getMessage());
            }

			if (!empty($treeid)) {
				$sql = '
					SELECT t.linkid 
					FROM sq_ast_lnk_tree t
					WHERE 
						t.treeid LIKE :treeid';

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					MatrixDAL::bindValueToPdo($query, 'treeid', $treeid.'%');
					$result = MatrixDAL::executePdoAssoc($query);
				} catch (Exception $e) {
					throw new Exception('Unable to get linkids like treeid: '.$treeid.' due to database error: '.$e->getMessage());
				}

				$result[]['linkid'] = '';
				$to_delete_links = Array();
				foreach($result as $row) {
					if (isset($row['linkid']) && $row['linkid']) {
						$to_delete_links[] = $row['linkid'];
					}
				}

				// Get rid of redendent links from the link and link tree table
				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				$sql = '
					DELETE FROM sq_ast_lnk
					WHERE linkid IN ('.implode(', ',$to_delete_links).')';

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					$result = MatrixDAL::executePdoAssoc($query);
				} catch (Exception $e) {
					throw new Exception('Unable to delete redundent links from sq_ast_lnk table due to database error: '.$e->getMessage());
				}

				$sql = '
					DELETE FROM sq_ast_lnk_tree
					WHERE linkid IN ('.implode(', ',$to_delete_links).')';

				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					$result = MatrixDAL::executePdoAssoc($query);
				} catch (Exception $e) {
					throw new Exception('Unable to delete redundent links from sq_ast_lnk_tree table due to database error: '.$e->getMessage());
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			}

			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		}// end if

	}//end function _abortAssetCreate()


	/**
	* Get the selectable create locations that the user has actually selected for the new asset
	*
	* @return array
	* @access protected
	*/
	function getSelectedCreateLocations()
	{
		$all_selected_locations = Array();
		foreach ($this->attr('create_location_optional') as $set_name => $details) {
			$selected_locations = Array();
			$code_name = $this->_sanitiseLocationSetName($set_name);
			if (isset($_POST[$code_name])) {
				if ($details['allow_multiple'] && is_array($_POST[$code_name])) {
					foreach ($_POST[$code_name] as $location_id) {
						// add it to the list
						$selected_locations[] = $location_id;
					}
					// check the list options are valid
					$selected_locations = array_intersect($selected_locations, array_keys($details['locations']));
					// add them to the collection
					foreach ($selected_locations as $location_id) {
						$all_selected_locations[$location_id] = (int)$details['link_type'];
					}
				} else {
					// if it's valid, add it to the list
					if (in_array($_POST[$code_name], array_keys($details['locations']))) {
						$all_selected_locations[$_POST[$code_name]] = (int)$details['link_type'];
					}
				}
			} else if (!$details['allow_empty']) {
				trigger_localised_error('CMS0036', E_USER_WARNING, $set_name);
				return FALSE;
			}
		}
		return $all_selected_locations;

	}//end getSelectedCreateLocations()


	/**
	* Get all the create locations that could ever be selected by the user, with their max link type
	*
	* @return array
	* @access protected
	*/
	function getAllSelectableCreateLocations()
	{
		$res = Array();
		$selectable_sets = $this->attr('create_location_optional');
		if (is_array($selectable_sets)) {
			foreach ($selectable_sets as $set_name => $set_details) {
				foreach ($set_details['locations'] as $loc => $name) {
					if (isset($res[$loc])) {
						$res[$loc] = min($res[$loc], $set_details['link_type']);
					} else {
						$res[$loc] = $set_details['link_type'];
					}
				}
			}
		}
		return $res;

	}//end getAllSelectableCreateLocations()


	/**
	* Get the IDs of the metadata schemas that should be applied to newly-created assets
	*
	* @return array
	* @access protected
	*/
	function _getMetadataSchemasToApply()
	{
		if (empty($this->id)) return Array();
		$schema_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, 'metadata_schema', FALSE, 'major', 'schema_to_apply');
		$res = Array();
		foreach ($schema_links as $link) {
			$res[] = $link['minorid'];
		}
		return $res;

	}//end _getMetadataSchemasToApply()


	/**
	* Get the fixed create locations
	*
	* Just included for completeness & consistency
	*
	* @return array
	* @access protected
	*/
	function getFixedCreateLocations()
	{
		return $this->attr('create_location');

	}//end getFixedCreateLocations()


	/**
	* Get the create locations determined by rule matches
	*
	* @param string	$create_type	The type of the asset being created
	* @param array	$attributes		The attributes of the new asset.  (If blank, attributes read from POST)
	*
	* @return array
	* @access protected
	*/
	function getRuleMatchCreateLocations($create_type, $attributes=Array())
	{
		$all_rule_match_locations = Array();
		if (empty($attributes)) {
			$create_type = $_POST['AB_'.$this->id.'_ASSET_BUILDER_CREATE_TYPE'];
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($create_type);
			$temp_asset = new $create_type();
			$prefix = $temp_asset->getPrefix();
			unset($temp_asset);
		}

		// check what rules dictate where the asset should be created
		$location_rules = $this->attr('create_location_rules');
		$attributeids = ($GLOBALS['SQ_SYSTEM']->am->getAssetTypeAttributes($create_type, Array('name', 'attrid')));
		if (!empty($location_rules[$create_type])) {
			foreach ($location_rules[$create_type] as $i => $rule) {
				$pass = TRUE;
				$j = 0;
				while ($j < count($rule['attributes'])) {
					if (empty($attributes)) {
						$attr_value = isset($_POST[$prefix.'_'.$attributeids[$rule['attributes'][$j]]['attrid']]) ? trim(strtolower($_POST[$prefix.'_'.$attributeids[$rule['attributes'][$j]]['attrid']])) : '';
					} else {
						$attr_value = isset($attributes[$rule['attributes'][$j]]) ? trim(strtolower($attributes[$rule['attributes'][$j]])) : '';
					}

					switch ($rule['comparators'][$j]) {
						case 'equals':
							if ($attr_value != $rule['values'][$j]) $pass = FALSE;
						break;

						case 'starts_with':
							if (!preg_match('|^'.$rule['values'][$j].'.*$|', $attr_value)) {
								$pass = FALSE;
							}
						break;

						case 'ends_with':
							if (!preg_match('|^.*'.$rule['values'][$j].'$|', $attr_value)) {
								$pass = FALSE;
							}
						break;

						case 'contains':
							if (strpos($attr_value, $rule['values'][$j]) === FALSE) {
								$pass = FALSE;
							}
						break;
					}
					$j++;
				}//end while

				if ($pass) {
					foreach ($rule['locations'] as $index => $assetid) {
						$all_rule_match_locations[$assetid] = $rule['link_types'][$index];
					}
				}
			}//end foreach
		}//end if
		return $all_rule_match_locations;

	}//end getRuleMatchCreateLocations()


	/**
	* Get the relevant create locations for a new asset
	*
	* @param string	$create_type	the type of asset we are getting create locations for
	*
	* @return array
	* @access protected
	*/
	function getCreateLocations($create_type)
	{
		if (is_array($selected_locs = $this->getSelectedCreateLocations())) {
			$res = $this->getFixedCreateLocations()
				+ $selected_locs
				+ $this->getRuleMatchCreateLocations($create_type)
				+ $this->getAdditionalCreateLocations()
				+ $this->getDynamicCreateLocations();

			// make sure all the create locations actually exist
			$details = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($res), Array(), TRUE, 'name');
			foreach ($res as $i => $v) {
				if (empty($details[$i])) {
					unset($res[$i]);
				}
			}

			return $res;
		} else {
			// failed to select some compulsory selectable locations
			return FALSE;
		}

	}//end getCreateLocations()


	/**
	* Get a list of additional create locations from the REQUEST var
	*
	* Also ensures the passed additional locations are valid by checking the allowed
	* additional create locations that have been set in the asset
	*
	* @return array
	* @access protected
	*/
	function getAdditionalCreateLocations()
	{
		$res = Array();
		$additional_locs = '';

		$parameter_map = $this->getAttribute('parameter_map');
		if (isset($_REQUEST['AB_'.$this->id.'_ASSET_BUILDER_ADDITIONAL_LOCATIONS']) && !empty($_REQUEST['AB_'.$this->id.'_ASSET_BUILDER_ADDITIONAL_LOCATIONS'])) {
			$additional_locs = $_REQUEST['AB_'.$this->id.'_ASSET_BUILDER_ADDITIONAL_LOCATIONS'];
		} else if ($parameter_map->getParameterValue('additional_locations') != NULL) {
			// As per Bug #2982, the POST var was not registering here, so here we check for the value
			// and allow asset builder to create the asset.
			$additional_locs = $parameter_map->getParameterValue('additional_locations');
		}

		if (!empty($additional_locs)) {
			// get the assetids and link types
			foreach (explode(';', $additional_locs) as $loc_code) {
				if (FALSE === strpos($loc_code, '-')) {
					// no link type specified, use the default
					$link_type = $this->attr('additional_location_default_link_type');
					$loc_id = $loc_code;
				} else {
					list($loc_id, $link_type) = explode('-', $loc_code);
				}
				if (!isset($res[$loc_id]) || ($res[$loc_id] > $link_type)) {
					$res[$loc_id] = $link_type;
				}
			}
			// check that the types and tree locations are valid
			$infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($res));
			$valid_types = $this->attr('additional_location_types');
			$valid_parents = $this->attr('additional_location_parents');
			foreach ($infos as $assetid => $info) {
				if (!in_array($info['type_code'], $valid_types)) {
					$matches = array_intersect($GLOBALS['SQ_SYSTEM']->am->getTypeAncestors($info['type_code']), $valid_types);
					if (empty($matches)) {
						unset($res[$assetid]);
					} else {
						// save us looking it up again
						$valid_types[] = $info['type_code'];
					}
				}
				if (!in_array($assetid, $valid_parents)) {
					$matches = array_intersect(array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($assetid)), $valid_parents);
					if (empty($matches)) unset($res[$assetid]);
				}
			}
		} // end if (!empty($additional_locs))
		return $res;

	}//end getAdditionalCreateLocations()


	/**
	* Get a list of dynamic create locations from the REQUEST var
	*
	* @return array
	* @access protected
	*/
	function getDynamicCreateLocations()
	{
		$all_selected_locations = Array();
		foreach ($this->attr('create_location_dynamic') as $set_name => $details) {
			$code_name = $this->_sanitiseLocationSetName($set_name);
			if (isset($_POST[$code_name]) && !empty($_POST[$code_name])) {
				$locations = $_POST[$code_name];
				if (!is_array($locations)) {
					$locations = Array($_POST[$code_name]);
				}

				foreach ($locations as $location_id) {
					$all_selected_locations[$location_id] = $details['link_type'];
					if (!$details['allow_multiple']) break;
				}
			} else if (!$details['allow_empty']) {
				trigger_localised_error('CMS0036', E_USER_WARNING, $set_name);
				return Array();
			}
		}

		return $all_selected_locations;

	}//end getDynamicCreateLocations()


	/**
	* Sanitises the passed create location set name
	*
	* @param string	$set_name	the name to sanitise
	*
	* @return string
	* @access private
	*/
	function _sanitiseLocationSetName($set_name)
	{
		return str_replace(Array(' ','\''), '_', strtolower($set_name));

	}//end _sanitiseLocationSetName()


	/**
	* Takes the default link that was created and allows for adjustments if necessary
	*
	* @param array	&$link	the current link array
	*
	* @return void
	* @access private
	*/
	function _modifyCreateLink(&$link)
	{
		return;

	}//end _modifyCreateLink()


	/**
	* Set the status of the newly created asset
	*
	* @param object	&$new_asset	the newly created asset
	*
	* @return boolean
	* @access private
	*/
	function _setCreateStatus(&$new_asset)
	{
		$create_status = $this->attr('create_status');
		if ($create_status != 0) {
			if ($new_asset->status != $create_status) {
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$vars = Array('assetid' => $new_asset->id, 'new_status' => $create_status);
				$status_errors = $hh->freestyleHipo('hipo_job_edit_status', $vars);

				//make sure the HIPO error message is in right format for asset builder
				$message = Array();
				foreach ($status_errors as $error) {
					if(is_array($error) && array_key_exists('message',$error)) {
						$message []= $error['message'];
					}
					else {
						$message = $error;
					}
				}

				$this->_errors = array_merge($this->_errors, $message);
				return empty($this->_errors);
			}
		}
		return TRUE;

	}//end _setCreateStatus()


	/**
	* Loads keyword replacements for the newly created asset, into the Asset Builder
	*
	* @return void
	* @access private
	*/
	function _loadAssetReplacements()
	{
		$this->_replacements['created_assetid'] 			= $this->getKeywordReplacement('created_assetid');
		$this->_replacements['created_asset_name'] 			= $this->getKeywordReplacement('created_asset_name');
		$this->_replacements['created_asset_short_name']	= $this->getKeywordReplacement('created_asset_short_name');
		$this->_replacements['created_asset_url']			= $this->getKeywordReplacement('created_asset_url');
		$this->_replacements['created_asset_edit']			= $this->getKeywordReplacement('created_asset_edit');
		$this->_replacements['created_asset_backend']		= $this->getKeywordReplacement('created_asset_backend');

	}//end _loadAssetReplacements()


	/**
	* Loads keyword replacements for the create asset interface, into the Asset Builder
	*
	* @return void
	* @access private
	*/
	function _loadCreateReplacements()
	{

		  ////////////////////////////
		 // CREATE CAPTCHA KEYWORD //
		////////////////////////////

		// Moved to Page_Asset_Builder::_printContents() as per Bug #5518: CAPTCHA settings conflict.

		  ////////////////////////////////
		 // CREATE MOLLOM SPAM KEYWORD //
		////////////////////////////////
		$regen_link_text = $this->attr('mollom_captcha_regen_link_text');
		if (empty($regen_link_text)) {
			$regen_link_text = translate('cms_asset_builder_mollom_regen_link_text');
		}
		$this->_replacements['asset_builder_mollom_captcha'] = Mollom::getMollomCaptchaKeywordReplacement($this->id);
		$this->_replacements['asset_builder_mollom_captcha_image'] = Mollom::getMollomCaptchaImageKeywordReplacement($this->id);
		$this->_replacements['asset_builder_mollom_captcha_audio'] = Mollom::getMollomCaptchaAudioKeywordReplacement($this->id);
		$this->_replacements['asset_builder_mollom_captcha_field'] = Mollom::getMollomCaptchaFieldKeywordReplacement($this->id);
		$this->_replacements['asset_builder_mollom_captcha_regen_link'] = Mollom::getMollomCaptchaRegenLinkKeywordReplacement($this->id, $regen_link_text);

		  ///////////////////////////
		 // GOOGLE RECAPTCHA 	  //
		///////////////////////////
		$this->_replacements['form_recaptcha'] = $this->getKeywordReplacement('form_recaptcha');

		  ///////////////////////////
		 // HONEYPOT QUESTION	  //
		///////////////////////////
		$this->_replacements['form_honeypot_field'] = $this->_getHoneypotField();

		  ///////////////////////////
		 // CREATE INVITE KEYWORD //
		///////////////////////////
		$this->_replacements['create_invite'] = $this->getKeywordReplacement('create_invite');

		  /////////////////////////
		 // CREATE FORM KEYWORD //
		////////////////////////
		$this->_replacements['create_form'] = $this->getKeywordReplacement('create_form');		

		  //////////////////////////////////////////
		 // SELECTABLE CREATE LOCATIONS KEYWORDS //
		//////////////////////////////////////////
		foreach ($this->attr('create_location_optional') as $name => $data) {
			$code_name = $this->_sanitiseLocationSetName($name);

			// the following line will remove any assets that no longer exist
			$options = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($data['locations']), Array(), TRUE, 'name');

			if (!$this->attr('allow_public_user')) {
				$remaining_options = Array();
				$user_groups = $GLOBALS['SQ_SYSTEM']->user->getUserGroups();
				$user_groups[] = $GLOBALS['SQ_SYSTEM']->user->id;
				$user_groups[] = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('public_user');
				foreach ($options as $key => $value) {
					$allowed_user_groups = $GLOBALS['SQ_SYSTEM']->am->getPermission($key,SQ_PERMISSION_WRITE,TRUE);
					// getPermission does not return root or sysadmin users so we will have to add the options ourselves
					// NOTE: root and sysadmin users are allowed to do anything unless _checkPermissionAccess is overridden
					if (array_intersect($user_groups,$allowed_user_groups)) {
						$remaining_options[$key] = $value;
					} else if (($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin())) {
						$remaining_options[$key] = $value;
					}
				}
				$options = $remaining_options;
			}
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
								label(str_replace(' ', '&nbsp;',$options[$id]), $code_name."[$i]");
								echo "<br />";
								$checked = FALSE;
								$i++;
							}
						} else {
							$checked = !$data['allow_empty'];

							foreach ($data['locations'] as $id => $name) {
								radio_button($code_name, $id, $checked);
								label(str_replace(' ', '&nbsp;',$options[$id]), $code_name.'_'.$id);
								echo "<br />";
								$checked = FALSE;
							}
						}
					break;

					default:
						echo '('.translate('cms_asset_builder_undefined_input_style', $data['input_style']).')';
					break;
				}//end switch
				$this->_replacements[$code_name.'_field'] = ob_get_contents();
			ob_end_clean();
		}//end foreach selectable create location

		foreach ($this->attr('create_location_dynamic') as $name => $data) {
			$code_name = $this->_sanitiseLocationSetName($name);

			// get the selected asset types and inherited types if necessary
			$types = Array();
			if (is_array($new_types = $data['types'])) {
				if (isset($new_types['type_code']) && isset($new_types['inherit'])) {
					if (count($new_types['type_code']) == count($new_types['inherit'])) {
						$new_types_keys = array_keys($new_types['type_code']);

						for ($ii = reset($new_types_keys); $ii < count($new_types_keys); $ii++) {
							if (empty($new_types['type_code'][$ii])) continue;
							if ($new_types['inherit'][$ii] == '1') {
								// get all descendants, add them to the types array
								$descendant_types = $GLOBALS['SQ_SYSTEM']->am->getTypeDescendants($new_types['type_code'][$ii], TRUE);
								foreach ($descendant_types as $type_code) {
									if (!isset($types[$type_code])) {
										$types[$type_code] = '';
									}
								}
							} else {
								$types[$new_types['type_code'][$ii]] = '';

							}
						}
					}
				}
			}
			$types = array_keys($types);
			ob_start();
				structured_drop_down($code_name, $data['root'], $types, Array(), 0, $data['allow_multiple'] ? 5 : 1, $data['depth'], $data['allow_multiple'], '-', FALSE, '', $data['input_style'] == 'radio', Array(), $data['display_structured'], array_keys($data['filter_link_type']), $data['allow_empty'], $this->attr('allow_public_user') ? NULL : SQ_PERMISSION_WRITE);
				$this->_replacements[$code_name.'_field'] = ob_get_contents();
			ob_end_clean();
		}//end foreach

		// Replace all available keywords in create_form keyword replacement, not leaving it to mysource.inc to pick it up
		replace_keywords($this->_replacements['create_form'], $this->_replacements);

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
		$bodycopy_data = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, $this->_current_state, 'major', '1');
		if (empty($bodycopy_data)) return;
		$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_data['minorid'], $bodycopy_data['minor_type_code']);
		if (is_null($bodycopy)) return;

		$keywords = $bodycopy->getKeywords();
		foreach($keywords as $word) {

			// Bug #5518: CAPTCHA settings conflict. Only replace if keyword exists in the content.
			if ($word == 'create_form' && isset($this->_replacements['create_form'])){
				$create_form_keywords = extract_keywords($this->_replacements['create_form']);
				$captcha_keywords = Array('form_captcha', 'form_captcha_image', 'form_captcha_field', 'form_captcha_link');
				$captcha_replacements = Array();
				foreach ($captcha_keywords as $captcha_word){
					if (in_array($captcha_word, $create_form_keywords)){
						$captcha_replacements[$captcha_word] = $this->getKeywordReplacement($captcha_word);
					}
				}
				replace_keywords($this->_replacements['create_form'], $captcha_replacements);
			}

			if ((strpos($word, 'form_captcha') === 0)){
				$this->_replacements[$word] = $this->getKeywordReplacement($word);
			}

			if ($this->attr('use_recaptcha') && strpos($word, 'form_recaptcha') === 0) {
				$this->_replacements[$word] = $this->getKeywordReplacement($word);
			}

			//pick up any global keywords and replace them
			if(!isset($this->_replacements[$word]) && (strpos($word, 'globals_') === 0 )){
				$this->_replacements[$word] = $this->getKeywordReplacement($word);
				// replace them here, it's not good enough to leave it to mysource.inc for replacement
				// mysource.inc will only do replacement in frontend, account manager will be used in simple edit mode,
				// so for global keywords in account manager we have to replace it here
				replace_global_keywords($this->_replacements[$word]);
			}
		}


		// Apply keyword modifiers to the keyword replacements
		foreach($keywords as $full_keyword) {
			$keyword = parse_keyword($full_keyword, $modifiers);
			if (!empty($modifiers) && isset($this->_replacements[$keyword])) {
				$this->_replacements[$full_keyword] = $this->_replacements[$keyword];
				apply_keyword_modifiers($this->_replacements[$full_keyword], $modifiers);
			}//end if
		}//end foreach

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
	* @return void
	* @access private
	* @see HIPO_Job::process()
	*/
	function _errorHandler($err_no, $err_msg, $err_file, $err_line)
	{
		$terminate = ((E_USER_ERROR | E_ERROR) & $err_no);

		// Treat PHP 5.2 catchable fatals as PHP 5.1 fatals
		if (defined('E_RECOVERABLE_ERROR') && ($err_no == E_RECOVERABLE_ERROR)) {
			$terminate = TRUE;
		}

		// if the function didn't have an '@' prepended OR if we are about to terminate
		// catch the error
		if ((error_reporting() & $err_no) || $terminate) {

			$bt = debug_backtrace();
			if (count($bt) > 1) {
				$real_bt_index = 0;
				while (($real_bt_index < count($bt)-1) && ((FALSE !== strpos(strtolower(array_get_index($bt[$real_bt_index], 'class')), 'locale_manager')) || (FALSE !== strpos(strtolower(array_get_index($bt[$real_bt_index], 'file')), 'locale_manager')) || (strtolower($bt[$real_bt_index]['function']) == '_errorhandler'))) {
					$real_bt_index++;
				}
				$err_file = hide_system_root($bt[$real_bt_index]['file']);
				$err_line = $bt[$real_bt_index]['line'];
				$err_msg  = hide_system_root($err_msg);
			}

			$text_msg = strip_tags(preg_replace(Array('/<br\\/?>/i', '/<p[^>]*>/i'), Array("\n", "\n\n"), $err_msg));

			// send a report to the system error log
			if (ini_get('log_errors')) {
				log_error($text_msg, $err_no, $err_file, $err_line);
			}

			$this->_errors[] = $text_msg;

		}//end error_reporting

		if ($terminate) exit(1);

	}//end _errorHandler()


	/**
	* Return an array of bodycopies that need to be created
	*
	* @return array
	* @access private
	*/
	function _getBodycopies()
	{
		$res = Array();
		$res['not_logged_in']= Array(
								'name'		=> translate('not_logged_in'),
								'content'	=> '%login_error%
												%login_invite%
												%login_form%',
							   );
		$res['logged_in']= Array(
							'name'		=> translate('logged_in'),
							'content'	=> '%create_error%
											%create_invite%
											%create_form%',
						   );
		$res['created']= Array(
							'name'		=> translate('created'),
							'content'	=> translate('cms_asset_builder_default_created_message'),
						 );
		return $res;

	}//end _getBodycopies()


//--        KEYWORD DESCRIPTION        --//


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access public
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) return;

		// which type-code?
		$parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id, 'bodycopy', TRUE);
		$type_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy');
		$bodycopy_type = '';
		foreach ($type_links as $link_info) {
			if (isset($parents[$link_info['minorid']])) {
				$bodycopy_type = $link_info['value'];
			}
		}

		switch ($bodycopy_type) {
			case 'not_logged_in' :
				$vars['keywords'] = array_merge($vars['keywords'], $this->_getNotLoggedInKeywords());
			break;
			case 'logged_in' :
				$vars['keywords'] = array_merge($vars['keywords'], $this->_getLoggedInKeywords());
			break;
			case 'created' :
				$vars['keywords'] = array_merge($vars['keywords'], $this->_getCreatedKeywords());
			break;
		}

		$vars['keywords'] = array_merge($vars['keywords'], $this->_getSelectableLocationKeywords());
		$vars['keywords'] = array_merge($vars['keywords'], $this->_getDynamicLocationKeywords());

	}//end onRequestKeywords()


	/**
	* Get the keyword used in not_logged_in bodycopy
	*
	* @return array
	* @access protected
	*/
	function _getNotLoggedInKeywords()
	{
		$keywords = Array();

		$keywords['login_invite'] 	= translate('cms_asset_builder_keyword_login_invite');
		$keywords['login_form'] 	= translate('cms_asset_builder_keyword_login_form');
		$keywords['login_error'] 	= translate('cms_asset_builder_keyword_login_error');
		$keywords['create_invite']	= translate('cms_asset_builder_keyword_create_invite');
		$keywords['create_form'] 	= translate('cms_asset_builder_keyword_create_form');
		$keywords['create_error'] 	= translate('cms_asset_builder_keyword_create_error');

		if ($this->attr('require_captcha')) {
			$keywords['form_captcha'] = translate('cms_asset_builder_keyword_form_captcha');
			$keywords['form_captcha_image'] = translate('cms_asset_builder_keyword_form_captcha_image');
			$keywords['form_captcha_field'] = translate('cms_asset_builder_keyword_form_captcha_field');
			$keywords['form_captcha_link'] = translate('cms_asset_builder_keyword_form_captcha_link');
		}

		if ($this->attr('use_mollom')) {
			$keywords['asset_builder_mollom_captcha']       = translate('cms_asset_builder_mollom_captcha');
			$keywords['asset_builder_mollom_captcha_image'] = translate('cms_asset_builder_mollom_captcha_image');
			$keywords['asset_builder_mollom_captcha_audio'] = translate('cms_asset_builder_mollom_captcha_audio');
			$keywords['asset_builder_mollom_captcha_field'] = translate('cms_asset_builder_mollom_captcha_field');
			$keywords['asset_builder_mollom_captcha_regen_link']  = translate('cms_asset_builder_mollom_captcha_link');
		}

		if ($this->attr('use_recaptcha')) {
			$keywords['form_recaptcha'] = translate('cms_asset_builder_form_recaptcha');
		}
		$keywords['form_honeypot_field'] = translate('cms_asset_builder_form_honeypot_field');

		return $keywords;

	}//end _getNotLoggedInKeywords()


	/**
	* Get the keyword used in logged_in bodycopy
	*
	* @return array
	* @access protected
	*/
	function _getLoggedInKeywords()
	{
		$keywords = Array();
		$keywords['create_invite']	= translate('cms_asset_builder_keyword_create_invite');
		$keywords['create_form'] 	= translate('cms_asset_builder_keyword_create_form');
		$keywords['create_error'] 	= translate('cms_asset_builder_keyword_create_error');

		if ($this->attr('require_captcha')) {
			$keywords['form_captcha'] = translate('cms_asset_builder_keyword_form_captcha');
			$keywords['form_captcha_image'] = translate('cms_asset_builder_keyword_form_captcha_image');
			$keywords['form_captcha_field'] = translate('cms_asset_builder_keyword_form_captcha_field');
			$keywords['form_captcha_link'] = translate('cms_asset_builder_keyword_form_captcha_link');
		}

		if ($this->attr('use_mollom')) {
			$keywords['asset_builder_mollom_captcha']       = translate('cms_asset_builder_mollom_captcha');
			$keywords['asset_builder_mollom_captcha_image'] = translate('cms_asset_builder_mollom_captcha_image');
			$keywords['asset_builder_mollom_captcha_audio'] = translate('cms_asset_builder_mollom_captcha_audio');
			$keywords['asset_builder_mollom_captcha_field'] = translate('cms_asset_builder_mollom_captcha_field');
			$keywords['asset_builder_mollom_captcha_regen_link']  = translate('cms_asset_builder_mollom_captcha_link');
		}

		if ($this->attr('use_recaptcha')) {
			$keywords['form_recaptcha'] = translate('cms_asset_builder_form_recaptcha');
		}
		$keywords['form_honeypot_field'] = translate('cms_asset_builder_form_honeypot_field');

		return $keywords;

	}//end _getLoggedInKeywords()


	/**
	* Get the keyword used in created bodycopy
	*
	* @return array
	* @access protected
	*/
	function _getCreatedKeywords()
	{
		$keywords = Array();

		$keywords['created_assetid']			= translate('cms_asset_builder_created_assetid');
		$keywords['created_asset_name']			= translate('cms_asset_builder_created_asset_name');
		$keywords['created_asset_short_name']	= translate('cms_asset_builder_created_asset_short_name');
		$keywords['created_asset_url']			= translate('cms_asset_builder_created_asset_url');
		$keywords['created_asset_edit']			= translate('cms_asset_builder_created_asset_edit');
		$keywords['created_asset_backend']		= translate('cms_asset_builder_created_asset_backend');

		return $keywords;

	}//end _getCreatedKeywords()


	/**
	* Get the keyword used to select selectable create locations
	*
	* @return array
	* @access protected
	*/
	function _getSelectableLocationKeywords()
	{
		$keywords = Array();
		foreach ($this->attr('create_location_optional') as $name => $data) {
			$keyword_name = $this->_sanitiseLocationSetName($name).'_field';
			$keywords[$keyword_name] = translate('cms_asset_builder_field_keyword', $name);
		}
		return $keywords;

	}//end _getSelectableLocationKeywords()


	/**
	* Get the keyword used to select dynamic create locations
	*
	* @return array
	* @access protected
	*/
	function _getDynamicLocationKeywords()
	{
		$keywords = Array();
		foreach ($this->attr('create_location_dynamic') as $name => $data) {
			$keyword_name = $this->_sanitiseLocationSetName($name).'_field';
			$keywords[$keyword_name] = translate('cms_asset_builder_field_keyword', $name);
		}
		return $keywords;

	}//end _getDynamicLocationKeywords()


//--        KEYWORD REPLACEMENT        --//


	/**
	* Get login_invite keyword replacement
	*
	* @return string
	* @access public
	*/
	function getLoginInviteKeywordReplacement()
	{
		return $this->attr('login_invite');

	}//end getLoginInviteKeywordReplacement()


	/**
	* Get login_invite keyword replacement
	*
	* @return string
	* @access public
	*/
	function getLoginErrorKeywordReplacement()
	{
		return $this->attr('login_error');

	}//end getLoginErrorKeywordReplacement()


	/**
	* Get login_form keyword replacement
	*
	* @return string
	* @access public
	*/
	function getLoginFormKeywordReplacement()
	{
		ob_start();
			?>
			<input type="hidden" name="SQ_LOGIN_KEY"      value="<?php echo $GLOBALS['SQ_SYSTEM']->generateLoginKey(); ?>" />
			<input type="hidden" name="SQ_LOGIN_REFERER"  value="" />

			<table border="0">
				<tr>
					<td valign="top"><?php echo translate('username'); ?></td>
					<td valign="top"><?php echo text_box('SQ_LOGIN_USERNAME', (isset($_POST['SQ_LOGIN_USERNAME'])) ? $_POST['SQ_LOGIN_USERNAME'] : 'Enter your username here', 25, 0, (isset($_POST['SQ_LOGIN_USERNAME'])) ? FALSE : TRUE, 'title="Username"'); ?></td>
				</tr>
				<tr>
					<td valign="top"><?php echo translate('password'); ?></td>
					<td valign="top"><?php echo password_box('SQ_LOGIN_PASSWORD', '', 25, 0, 'title="Password"'); ?></td>
				</tr>
				<tr>
					<td valign="top">&nbsp;</td>
					<td valign="top">
						<input type="hidden" name="SQ_ACTION" value="" id="SQ_ACTION" />
						<?php echo submit_button('SQ_LOGIN_SUBMIT', 'Login', 'document.getElementById(\'SQ_ACTION\').value = \'login\'; document.getElementById(\'AB_'.$this->id.'_ASSET_BUILDER_ACTION\').value = \'login\';'); ?>
					</td>
				</tr>
			</table>

			<?php
			$login_form = ob_get_contents();
		ob_end_clean();
		return $login_form;

	}//end getLoginFormKeywordReplacement()


	/**
	* Get created_assetid keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCreatedAssetidKeywordReplacement()
	{
		if(isset($this->_replacements['created_assetid'] )) {
		    return $this->_replacements['created_assetid'];
		}
		else if (isset($this->_tmp['created_asset'])) {
		    return $this->_tmp['created_asset']->id;
		}
		return '';
	}//end getCreatedAssetidKeywordReplacement()


	/**
	* Get created_asset_name keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCreatedAssetNameKeywordReplacement()
	{
		if(isset($this->_replacements['created_asset_name'] )) {
		    return $this->_replacements['created_asset_name'];
		}
		else if (isset($this->_tmp['created_asset'])) {
		    return $this->_tmp['created_asset']->name;
		}
		return '';

	}//end getCreatedAssetNameKeywordReplacement()


	/**
	* Get created_asset_short_name keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCreatedAssetShortNameKeywordReplacement()
	{
		if(isset($this->_replacements['created_asset_short_name'] )) {
		    return $this->_replacements['created_asset_short_name'];
		}
		else if (isset($this->_tmp['created_asset'])) {
		    return $this->_tmp['created_asset']->short_name;
		}
		return '';
	}//end getCreatedAssetShortNameKeywordReplacement()


	/**
	* Get created_asset_url keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCreatedAssetUrlKeywordReplacement()
	{
		if(isset($this->_replacements['created_asset_url'] )) {
		    return $this->_replacements['created_asset_url'];
		}
		else if (isset($this->_tmp['created_asset'])) {
		    return $this->_tmp['created_asset']->getURL();
		}
		return '';
	}//end getCreatedAssetUrlKeywordReplacement()


	/**
	* Get created_asset_edit keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCreatedAssetEditKeywordReplacement()
	{
		if(isset($this->_replacements['created_asset_edit'] )) {
		    return $this->_replacements['created_asset_edit'];
		}
		else if (isset($this->_tmp['created_asset'])) {
		    return $this->_tmp['created_asset']->getURL().'/'.SQ_CONF_LIMBO_SUFFIX;
		}
		return '';
	}//end getCreatedAssetEditKeywordReplacement()


	/**
	* Get created_asset_backend keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCreatedAssetBackendKeywordReplacement()
	{
		if(isset($this->_replacements['created_asset_backend'] )) {
		    return $this->_replacements['created_asset_backend'];
		}
		else if (isset($this->_tmp['created_asset'])) {
		    return sq_web_path('root_url').'/'.$this->_tmp['created_asset']->getBackendHref('contents');
		}
		return '';
	}//end getCreatedAssetBackendKeywordReplacement()


	/**
	* Get create_error keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCreateErrorKeywordReplacement()
	{
		return $this->attr('create_error');

	}//end getCreateErrorKeywordReplacement()


	/**
	* Get create_invite keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCreateInviteKeywordReplacement()
	{
		return $this->attr('create_invite');

	}//end getCreateInviteKeywordReplacement()


	/**
	* Get reCAPTCHA replcement
	*
	* @return string
	* @access public
	*/
	function getFormRecaptchaKeywordReplacement()
	{
		$replacement = '';
		if ($this->attr('use_recaptcha')) {
			$replacement =  Recaptcha::getRecaptcha(Array(), $this->attr('recaptcha_theme'));
		}

		return $replacement;

	}//end getFormRecaptchaKeywordReplacement()


	/**
	* Get create_form keyword replacement
	*
	* @return string
	* @access public
	*/
	function getCreateFormKeywordReplacement()
	{
		$create_form = '';
		$create_types = $this->attr('create_type');
		if (!is_array($create_types)) {
			$create_types = Array($create_types => '');
		}

		require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
		$o = new Limbo_Outputter();
		$o->addHiddenField('AB_'.$this->id.'_ASSET_BUILDER_CREATE_TYPE', key($create_types)); // first asset type is selected initially
		if (count($create_types) > 1) $o->addOnSubmit('disableHiddenInputs();');

		// Get the Metadata schemas ready
		$metadata_schemas = $this->_getMetadataSchemasToApply();
		$metadata_edit_fns = NULL;

		// get each asset type's form fields for the create form
		ob_start();
			$display = 'block'; // first create type is shown initially
			foreach ($create_types as $create_type => $value) {
				$prefix = $this->type().'_'.$this->id.'_type_'.$create_type;
				echo '<div class="sq-asset-builder-tab-content" id="'.$prefix.'" style="display: '.$display.'">';
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($create_type);
				$asset =  NULL;
				if (isset($this->_tmp['new_asset_created_'.$create_type])) {
					$asset =& $this->_tmp['new_asset_created_'.$create_type];
				} else {
					$asset = new $create_type();
				}

				if ($layout = $this->_getCreateFormLayout($create_type)) {
					$ei = $asset->getEI();
					$ei->paintLayout($asset, $o, $layout);
					$o->addHiddenField('asset_action', 'create_custom');
				} else {
					unset($GLOBALS['painted_inline_value_interface']);
					foreach ($metadata_schemas as $schemaid) {
						$schema = $GLOBALS['SQ_SYSTEM']->am->getAsset($schemaid);
						if (is_null($metadata_edit_fns)) {
							$metadata_edit_fns = $schema->getEditFns();
						}
						$metadata_edit_fns->paintInlineValueInterface($schema, $o, Array(), TRUE, FALSE, $create_type);
					}
					$asset->paintBackend($o);
					$o->addHiddenField('asset_action', 'create');
				}
				$o->paintSections();
				$o->_contents = Array();

				echo '</div>';

				// If this form was commited before but AB failed to create the asset due to errors
				// then we will insert the previously entered field values to the asset's attribute values
				?>
				<script type="text/javascript">
				<?php
					foreach ($asset->vars as $attr_name => $attr_details) {
						foreach ($_POST as $post_var => $post_var_value) {
							$pos = strpos($post_var, $create_type);
							if (($pos !== FALSE) && ($pos == 0)) {
								// does this var ends with the current attrid?
								$idlength = strlen($attr_details['attrid']);
								if (substr($post_var, -$idlength) == $attr_details['attrid']) {
									// we got the attribute value in post change the attr val
									?>
									document.getElementById('<?php echo htmlspecialchars($post_var, ENT_QUOTES); ?>').value = '<?php echo htmlspecialchars($post_var_value, ENT_QUOTES); ?>';
									<?php
								}
							}
						}
					}
				?>
				</script>
				<?php

				$display = 'none'; // all subsequent create types hidden initially
			}//end foreach
			$create_form = ob_get_contents();
		ob_end_clean();

		// get the header for the create form
		ob_start();
			// Header material from the limbo outputter
			$o->_paintInlineHeader();

			// Tabs if necessary
			if (count($create_types) > 1) {
				reset($create_types);
				?>
				<script type="text/javascript"><!--
				//<![CDATA[
					var current_option = '<?php echo $this->type().'_'.$this->id.'_type_'.key($create_types) ?>';

					function showAssetBuilderTab(type_code)
					{
						document.getElementById(current_option).style.display = 'none';
						document.getElementById(current_option+"_tab").className = 'sq-asset-builder-tab';
						current_option = '<?php echo $this->type().'_'.$this->id.'_type_' ?>' + type_code;
						document.getElementById(current_option).style.display = 'block';
						document.getElementById(current_option+"_tab").className = 'sq-asset-builder-tab-active';
						document.getElementById('AB_<?php echo $this->id ?>_ASSET_BUILDER_CREATE_TYPE').value = type_code;

					}//end showAssetBuilderTab()

					function disableHiddenInputs()
					{
						var divs = document.getElementsByTagName('DIV');
						for (var i=0; i < divs.length; i++) {
							if (divs[i].className == 'sq-asset-builder-tab-content') {
								if (divs[i].style.display == 'none') {
									var inputs = divs[i].getElementsByTagName('INPUT');
									for (var j=0; j < inputs.length; j++) {
										inputs[j].disabled = true;
									}
									inputs = divs[i].getElementsByTagName('TEXTAREA');
									for (var j=0; j < inputs.length; j++) {
										inputs[j].disabled = true;
									}
									inputs = divs[i].getElementsByTagName('SELECT');
									for (var j=0; j < inputs.length; j++) {
										inputs[j].disabled = true;
									}
								}
							}
						}
						return true;
					}

				//]]> -->
				</script>

				<div id="sq-asset-builder-header">
					<ul>
					<?php
					$active = '-active'; // first asset type is shown initially
					foreach ($create_types as $create_type => $value) {
						$prefix = $this->type().'_'.$this->id.'_type_'.$create_type;
						$type_name = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($create_type, 'name');
						?>
							<li class="sq-asset-builder-tab<?php echo $active; ?>" id="<?php echo $prefix; ?>_tab">
								<a href="#" onclick="showAssetBuilderTab('<?php echo $create_type ?>'); return false;">
									<?php echo $type_name; ?>
								</a>
							</li>
						<?php
						$active = ''; // subsequent asset types are hidden initially
					}
					?>
					</ul>
				</div>
				<?php
			}//end if
			$create_form = ob_get_contents().$create_form;
		ob_end_clean();

		return $create_form;

	}//end getCreateFormKeywordReplacement()


	/**
	* Get the create form layout, if available, for a certain create type
	*
	* @param string	$type_code	The type code to get the layout for
	*
	* @return object
	* @access private
	*/
	function &_getCreateFormLayout($type_code)
	{
		$res = NULL;
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_TYPE_2, 'layout', TRUE, $type_code);
		if (!empty($link)) {
			$res = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], 'layout');
		}

		return $res;

	}//end _getCreateFormLayout()


	/**
	* Get the Honeypot field
	* This field must not be visible to the user
	*
	* @return boolean
	* @access private
	*/
	function _getHoneypotField()
	{
		$name = $this->attr('honeypot_name');
		$id = $this->attr('honeypot_id');
		$class = $this->attr('honeypot_class');

		if (empty($name)) {
			$name = 'q'.$this->id.':comment';
		}
		if (empty($id)) {
			$id = 'q'.$this->id.'_comment';
		}

		$extras = 'class="'.$class.'"';
		$extras = $extras.' id="'.$id.'"';
		ob_start();
			text_box($name, '', 10, 10, FALSE, $extras);

		return ob_get_clean();

	}//end _getHoneypotField()


	/**
	* Returns TRUE if the Honeypot field was left unanswered
	* A genuine human user making the form submission will
	* never answer the Honeypot question
	*
	* @return boolean
	* @access private
	*/
	function _honeypotFieldEmpty()
	{
		$name = $this->attr('honeypot_name');
		if (empty($name)) {
			$name = 'q'.$this->id.':comment';
		}

		return empty($_POST[$name]);

	}//end _honeypotFieldEmpty()


	/**
	* Returns TRUE if the time since the form page load
	* has passed the minimum time threshold set
	*
	* @return boolean
	* @access private
	*/
	function _passedMinimumTimeThreshold()
	{
		$time_threshold = $this->attr('time_requirement');
		if (!$time_threshold || !is_numeric($time_threshold) || $time_threshold <= 0) {
			return TRUE;
		}

		// Get the form load time from the session
		$load_time = array_get_index($_SESSION, 'SQ_AB_'.$this->id.'_LOAD_TIME', 0);
		// Current time, i.e. submission time
		$now = time();

		if (($now-$load_time) < $time_threshold) {
			return FALSE;
		}

		return TRUE;

	}//end _passedMinimumTimeThreshold()

}//end class

?>
