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
* $Id: page_asset_builder_management.inc,v 1.44 2013/08/23 01:15:37 akarelia Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Asset_Builder_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.44 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Asset_Builder_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{

		$this->Asset_Management($pm);

		/**
		* Array of asset type codes that this page can create
		*/
		$this->vars['create_type'] = Array(
										'added'		=> '0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
									 );
		$this->vars['edit_type'] = Array(
										'added'		=> '0.1',
										'type'		=> 'serialise',
										'default'	=> Array(),
									 );

		$this->vars['create_form_customisations'] = Array(
														'added'		=> '0.8',
														'type'		=> 'serialise',
														'default'	=> Array(),
													);
		$this->vars['edit_form_customisations'] = Array(
														'added'		=> '0.8',
														'type'		=> 'serialise',
														'default'	=> Array(),
													);

		$this->vars['create_status'] = Array(
										'added'		=> '0.1',
										'type'		=> 'int',
										'default'	=> 2,
									   );

		$this->vars['required_fields'] = Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										 );

		$this->vars['link_value'] = Array(
										'added'		=> '0.8',
										'type'		=> 'text',
										'default'	=> '',
									);

		/**
		* Locations under which created assets will always be linked
		*	Array (assetid => link_type)
		*/
		$this->vars['create_location'] = Array(
											'added'		=> '0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										 );



		/**
		* Sets of locations for the user to choose from when creating assets, with selection options
		*	Array (name => Array('locations'		=> Array (assetid => name),
		*						  'allow_multiple'	=> 0 | 1,
		*						  'allow_empty'		=> 0 | 1,
		*						  'input_style'		=> 'dropdown' | 'radio',
		*						  'link_type'		=> link_type
		*						 )
		*		  )
		*/
		$this->vars['create_location_optional'] = Array(
													'added'		=> '0.1',
													'type'		=> 'serialise',
													'default'	=> Array(),
												  );

		/**
		* Dynamically-generated sets of locations for the user to choose from when creating assets
		*   Array(
		*		'set_name' => Array(
		*						'root'					=> location_id, 					// from asset_finder
		*						'depth'					=> 0,								// default to infinite levels
		*						'types'					=> Array(							// follows the format of the asset_type_selector
		*													'type_code'	=> Array(
		*																	0 => 'sample_type_code',
		*																   ),
		*													'inherit'	=> Array(
		*																	0 => '1',
		*																   ),
		*						'filter_link_type'		=> Array(),							// filter the locations list with this/these link_type(s). excludes the selected link types, none selected by default
		*						'link_types'			=> SQ_LINK_TYPE_1,					// create the asset and link it with this link_type
		*						'input_style'			=> 'dropdown',						// or 'radio'
		*						'display_structured'	=> 0								// Whether the dropbox or radio displays a tree, or a flat list of asset names
		*						'allow_multiple'		=> false, 							// boolean
		*						'allow_empty'			=> false, 							// boolean
		*					  )
		*   )
		*/
		$this->vars['create_location_dynamic'] = Array(
													'added'		=> '0.7',
													'type'		=> 'serialise',
													'default'	=> Array(),
												 );

		/**
		* Rules governing additional create locations
		*   Array (create_type => Array (index => Array('attributes'  => Array(index => attribute_name),
		*												'comparators' => Array(index => comparator_name),
		*												'values'	  => Array(index => comparison_value),
		*
		*												'locations'	  => Array(index => assetid),
		*												'link_types'  => Array(index => link_type)
		*											   )
		*								)
		*		  )
		*/
		$this->vars['create_location_rules'] = Array(
												'added'		=> '0.1',
												'type'		=> 'serialise',
												'default'	=> Array(),
											   );

		/**
		* WYSIWYGs
		*/
		$this->vars['login_invite'] = Array(
										'added'		=> '0.1',
										'type'		=> 'wysiwyg',
										'default'	=> '<p>Sign in using the form below.</p>',
										'is_contextable'	=> TRUE,
									  );
		$this->vars['login_error'] = Array(
										'added'		=> '0.1',
										'type'		=> 'wysiwyg',
										'default'	=> '<p>Incorrect username or password entered. Please try again.</p>',
										'is_contextable'	=> TRUE,
									 );
		$this->vars['create_invite'] = Array(
										'added'		=> '0.1',
										'type'		=> 'wysiwyg',
										'default'	=> '<p>Create an asset using the form below.</p>',
										'is_contextable'	=> TRUE,
									   );
		$this->vars['create_error'] = Array(
										'added'		=> '0.1',
										'type'		=> 'wysiwyg',
										'default'	=> '<p>The new asset was not created. Ensure you have entered all required information and try again.</p>',
										'is_contextable'	=> TRUE,
									  );
		/* Edit details attributes */
		$this->vars['edit_invite']	= Array(
											'added'		=> '0.1',
											'type'		=> 'wysiwyg',
											'default'	=> '<p>You are currently in edit mode.</p>',
											'is_contextable'	=> TRUE,
										);
		$this->vars['edit_lock_error']	= Array(
											'added'		=> '0.1',
											'type'		=> 'wysiwyg',
											'default'	=> '<p>Failed to acquire lock for editing #%1$s.</p>',
											'is_contextable'	=> TRUE,
										  );
		$this->vars['edit_success']		= Array(
											'added'		=> '0.1',
											'type'		=> 'wysiwyg',
											'default'	=> '<p>The asset has been updated.</p>',
											'is_contextable'	=> TRUE,
										  );
		$this->vars['edit_error'] = Array(
										'added'		=> '0.1',
										'type'		=> 'wysiwyg',
										'default'	=> '<p>The new asset was not updated. Ensure you have entered all required information and have permission to edit the asset before trying again.</p>',
										'is_contextable'	=> TRUE,
									  );
		/* Additional (query-fed) create locations must be children of one of these */
		$this->vars['additional_location_parents'] = Array(
														'added'		=> '0.5',
														'type'		=> 'serialise',
														'default'	=> Array(),
													 );

		/* Additional (query-fed) create locations will use this link type by default */
		$this->vars['additional_location_default_link_type'] = Array(
																'added'			=> '0.8',
																'type'			=> 'selection',
																'default'		=> SQ_LINK_TYPE_1,
																'parameters'	=> Array(
																					'options'	=> Array(
																									SQ_LINK_TYPE_1	=> 'TYPE_1',
																									SQ_LINK_TYPE_2	=> 'TYPE_2',
																									SQ_LINK_TYPE_3	=> 'TYPE_3',
																									SQ_LINK_NOTICE	=> 'NOTICE',
																								   ),
																				   ),
																'multiple'		=> FALSE,
																'allow_empty'	=> FALSE,
															   );

		/* Additional (query-fed) create locations must be of one of these types */
		$this->vars['additional_location_types'] = Array(
													'added'		=> '0.5',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   );

		/* Map for connecting runtime data sources with parameters */
		$this->vars['parameter_map'] = Array(
										'added'			=> '0.6',
										'type'			=> 'parameter_map',
										'parameters'	=> Array(
															'parameters'	=> Array(
																				'additional_locations'	=> 'Additional Create Locations',
																			   ),
														   ),
										'default'		=> Array(),
									   );
		
		$this->vars['edit_parameter_map'] = Array(
										'added'			=> '0.6',
										'type'			=> 'parameter_map',
										'parameters'	=> Array(
															'parameters'	=> Array(
																				'edit'	=> 'Edit Asset',
																			   ),
														   ),
										'default'		=> Array(),
									   );

		$this->vars['allow_public_user'] = Array(
											'added'			=> '0.1',
											'type'			=> 'boolean',
											'default'		=> TRUE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										   );

		$this->vars['submit_to_page_url'] = Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											);
		$this->vars['require_captcha'] = Array(
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'	=> FALSE,
															   ),
										 );
		$this->vars['captcha_length'] = Array(
											'added'		=> '0.9',
											'type'		=> 'int',
											'default'	=> 4,
										);
		$this->vars['captcha_zoom'] = Array(
										'added'		=> '0.9',
										'type'		=> 'int',
										'default'	=> 1,
									  );
		$this->vars['captcha_link_text'] = Array(
											'added'		=> '0.9',
											'type'		=> 'text',
											'default'	=> 'Regenerate security key',
										   );

		$this->vars['captcha_bgcolour']	= Array(
											'added'		=> '0.9',
											'type'		=> 'colour',
											'default'	=> 'FFFFFF',
										  );

		$this->vars['captcha_textcolour']	= Array(
												'added'		=> '0.9',
												'type'		=> 'colour',
												'default'	=> '000000',
											  );

		$this->vars['captcha_bordercolour'] = Array(
												'added'		=> '0.9',
												'type'		=> 'colour',
												'default'	=> '000000',
											  );

		$this->vars['captcha_use_colours']	= Array(
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											  );

		$this->vars['captcha_usefont']	= Array(
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array(
																'allow_empty'=> FALSE,
															   ),
										  );

		$this->vars['captcha_ttf']	= Array(
										'added'			=> '0.9',
										'type'			=> 'assetid',
										'default'		=> '',
										'description'	=> 'The asset id of the font file.',
									  );

		$this->vars['captcha_font_size'] = Array(
											'added'		=> '0.9',
											'type'		=> 'int',
											'default'	=> 20,
										   );

		$this->vars['captcha_min_angle'] = Array(
											'added'		=> '0.9',
											'type'		=> 'int',
											'default'	=> -20,
										   );

		$this->vars['captcha_max_angle'] = Array(
											'added'		=> '0.9',
											'type'		=> 'int',
											'default'	=> 20,
										   );

		$this->vars['captcha_x_start'] = Array(
											'added'		=> '0.9',
											'type'		=> 'int',
											'default'	=> 10,
										 );

		$this->vars['captcha_min_distance'] = Array(
												'added'		=> '0.9',
												'type'		=> 'int',
												'default'	=> 10,
											  );

		$this->vars['captcha_max_distance'] = Array(
												'added'		=> '0.9',
												'type'		=> 'int',
												'default'	=> 20,
											  );

		$this->vars['captcha_width'] = Array(
										'added'		=> '0.9',
										'type'		=> 'int',
										'default'	=> 200,
									   );

		$this->vars['captcha_height'] = Array(
											'added'		=> '0.9',
											'type'		=> 'int',
											'default'	=> 36,
										);

		$this->vars['captcha_arc'] = Array(
										'type'=> 'boolean',
										'default'=> FALSE,
										'parameters'=> Array(
														'allow_empty'=> FALSE,
													   ),
									 );

		$this->vars['captcha_arc_colour'] = Array(
												'added'=> '0.9',
												'type'=> 'colour',
												'default'=> 'D20A0A',
											);

		$this->vars['captcha_include_uppercase'] = Array(
													'type'=> 'boolean',
													'default'=> FALSE,
													'parameters'=> Array(
																	'allow_empty'=> FALSE,
																   ),
												   );

		$this->vars['captcha_include_numbers'] = Array(
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array(
																		'allow_empty'	=> FALSE,
																	   ),
												 );

		$this->vars['provide_accessible_captcha'] = Array(
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																		   ),
													 );

		$this->vars['captcha_invalid_error_message']		= Array(
																'added'			=> '0.9',
																'type'			=> 'text',
																'default'		=> 'Security Key Incorrect',
																'is_contextable'=> TRUE,
															  );

		$this->vars['captcha_label_text']					= Array(
																'added'			=> '0.9',
																'type'			=> 'text',
																'default'		=> 'Type the characters you see in the picture above: ',
																'is_contextable'=> TRUE,
															  );



		$this->vars['layout_to_apply'] = Array(
											'added'		=> '0.9',
											'type'		=> 'assetid',
											'default'	=> 0,
										 );
		$this->vars['override_layout_to_apply'] = Array(
													'added'		=> '0.9',
													'type'		=> 'assetid',
													'default'	=> 0,
												  );

		// Mollom Spam Check attributes ///////
		$this->vars['use_mollom'] 						= Array(
												'added'			=> '0.9',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'	=> Array(
																	'allow_empty'	=> FALSE,
																   ),
											);
		$this->vars['mollom_attributes_to_check'] 	= Array(
												'added'			=> '0.9',
												'type'			=> 'serialise',
												'default'		=>  Array(),
												'description'	=> 'Asset attributes to check for spam',
											);
		$this->vars['mollom_unsure_submission_text'] 	= Array(
												'added'			=> '0.9',
												'type'			=> 'text',
												'default'		=> 'Please enter the characters in the image to verify your submission.',
												'description'	=> 'Mollom error message text for unsure submission',
											);
		$this->vars['mollom_spam_submission_text'] 		= Array(
												'added'			=> '0.9',
												'type'			=> 'text',
												'default'		=> 'This submission looks like a spam. Cannot accept this submission.',
												'description'	=> 'Mollom error message text for spam submission',
											);
		$this->vars['mollom_captcha_incorrect_text'] 		= Array(
												'added'			=> '0.9',
												'type'			=> 'text',
												'default'		=> 'Incorrect security key. Please try again.',
												'description'	=> 'Mollom error message text for incorrect captcha',
											);
		$this->vars['mollom_captcha_regen_link_text'] 	= Array(
												'added'			=> '0.9',
												'type'			=> 'text',
												'default'		=> 'Regenerate security key',
												'description'	=> 'Mollom CAPTCHA regen link text',
											);
		$this->vars['use_recaptcha'] = 				Array(
														'added'			=> '0.9',
														'type'			=> 'boolean',
														'default'		=> FALSE,
														'parameters'	=> Array(
																			'allow_empty'	=> FALSE,
																				),
													);
		$this->vars['recaptcha_theme'] = 			Array(
														'added'			=> '0.9',
														'type'			=> 'selection',
														'default'		=> 'red',
														'parameters'	=> Array(
																			'multiple'		=> FALSE,
																			'allow_empty'   => FALSE,
																			'options'       => Array(
																								'red'	=> 'Red',
																								'white' => 'White',
																								'blackglass' => 'Black Glass',
																								'clean'	=> 'Clean',
																								),
																				),
													);
		$this->vars['recaptcha_error_message'] = 	Array(
														'added'		=> '0.9',
														'type'		=> 'text',
														'default'	=> '',
														'description' => 'Error message when Recaptcha validation fails',
													);
		$this->vars['honeypot_name'] = 				Array(
														'added'		=> '0.9',
														'type'		=> 'text',
														'default'	=> '',
														'description' => 'Honeypot input field name',
													);
		$this->vars['honeypot_id']	= 				Array(
														'added'		=> '0.9',
														'type'		=> 'text',
														'default'	=> '',
														'description' => 'Honeypot input field id',
													);
		$this->vars['honeypot_class'] =				Array(
														'added'		=> '0.9',
														'type'		=> 'text',
														'default'	=> '',
														'description' => 'Honeypot input field class name',
													);
		$this->vars['honeypot_error_message'] = 	Array(
														'added'		=> '0.9',
														'type'		=> 'text',
														'default'	=> '',
														'description' => 'Error message when Honeypot validation fails',
													);
		$this->vars['time_requirement']	=				Array(
															'added'		=> '0.9',
															'type'		=> 'text',
															'default'	=> '0',
															'description' => 'Time in seconds required to complete the form by a human user',
														);
		$this->vars['time_requirement_error_message'] = Array(
															'added'		=> '0.9',
															'type'		=> 'text',
															'default'	=> '',
															'description' => 'Error message when time requirement check fails',
														);
		$this->vars['password_autocomplete']		  = Array(
															'added'			=> '0.9',
															'type'			=> 'boolean',
															'default'		=> FALSE,
															'parameters'	=> Array(
																				'allow_empty'	=> FALSE,
																			   ),
														  );

		$this->vars['enforce_user_login'] = Array(
															'added'			=> '0.1',
															'type'			=> 'boolean',
															'default'		=> FALSE,
															'parameters'	=> Array(
																				'allow_empty'	=> FALSE,
																			   ),
														);

	}//end constructor


	/**
	* Return an array with information about the events that the account manager is listening for
	*
	* @return boolean
	* @access public
	*/
	function getEventList()
	{
		return Array(
				Array(
					'event_name'					=> 'requestKeywords',
					'broadcast_type_code'			=> 'content_type',
					'broadcast_strict_type_code'	=> FALSE,
					'options'						=> Array(
														'side_of_link'	=> 'major',
														'indirect'		=> TRUE,
														'is_exclusive'	=> NULL,
														'is_dependant'	=> 1,
														'value'			=> '',
														'link_type'		=> SQ_LINK_TYPE_2 | SQ_LINK_TYPE_1,
													   ),
				),
			   );

	}//end getEventList()


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		// important upgrades that must be taken care of before create type becomes
		// a serialise in 0.3
		if (version_compare($current_version, '0.2', '<=')) {
			$root_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_folder->id, $this->getAssetType(), FALSE);
			foreach ($children as $assetid => $type) {

				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type);
				if (is_null($asset)) continue;

				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes')) {
					trigger_localised_error('CMS0025', sprintf(translate('Asset Builder Upgrade [from %1$s] FAILED - Unable to Acquire Lock on "%2$s" (#%3$s)', '0.2'), $asset->name, $asset->id), E_USER_WARNING);
					return FALSE;
				}

				$old_create_type = $asset->attr('create_type');
				$create_type = Array($old_create_type => TRUE);
				$asset->setAttrValue('create_type', serialize($create_type));

				if (!$asset->saveAttributes()) {
					trigger_localised_error('CMS0026', sprintf(translate('Asset Builder Upgrade [from %s] FAILED'), '0.2'), E_USER_WARNING);
					return FALSE;
				}

				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'attributes');

			}
		}


		if (!parent::_upgrade($current_version)) return FALSE;

		if ($current_version == '0.1') {
			// this upgrade changed the attribute 'create_locations' to include additional
			// rules that meant the create locations has to be stored in a different format

			$root_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_folder->id, $this->getAssetType(), FALSE);
			foreach ($children as $assetid => $type) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type);
				if (is_null($asset)) continue;

				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes')) {
					trigger_localised_error('CMS0025', sprintf(translate('Asset Builder Upgrade [from %1$s] FAILED - Unable to Acquire Lock on "%2$s" (#%3$s)', '0.1'), $asset->name, $asset->id), E_USER_WARNING);
					return FALSE;
				}

				// 0.1->0.2 upgrades
				$new_locations = Array();
				$old_locations = $asset->attr('create_location_rules');

				$i = 0;
				foreach ($old_locations as $rule_text => $locations) {
					if (empty($rule_text)) continue;
					$bits = explode(' && ', $rule_text);

					foreach ($bits as $bit) {
						list($attr, $value) = explode('==', $bit);
						$attr  = trim($attr, '$');
						$value = trim($value, " '");
						$new_locations[$i][$attr]['equals'] = $value;
					}

					$new_locations[$i]['_create_locations_'] = array_keys($locations);

					$i++;
				}

				$asset->setAttrValue('create_location_rules', $new_locations);

				if (!$asset->saveAttributes()) {
					trigger_localised_error('CMS0026', sprintf(translate('Asset Builder Upgrade [from %s] FAILED'), '0.1'), E_USER_WARNING);
					return FALSE;
				}
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'attributes');
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}//end foreach

			pre_echo('ASSET BUILDER UPGRADE COMPLETE - VERSION 0.1');
		}//end if version 0.1 upgrade

		if ($current_version == '0.2') {
			// this update changed the way create types are stored, to allow for the creation
			// of multiple asset types
			pre_echo('STARTING ASSET BUILDER UPGRADE - VERSION 0.2');

			$root_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_folder->id, $this->getAssetType(), FALSE);
			foreach ($children as $assetid => $type) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type);
				if (is_null($asset)) continue;

				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes')) {
					trigger_localised_error('CMS0025', sprintf(translate('Asset Builder Upgrade [from %1$s] FAILED - Unable to Acquire Lock on "%2$s" (#%3$s)', '0.2'), $asset->name, $asset->id), E_USER_WARNING);
					return FALSE;
				}

				$create_locs = $asset->attr('create_location_rules');
				$create_locs = Array($old_create_type => $create_locs);

				$asset->setAttrValue('create_location_rules', $create_locs);

				$required_fields = $asset->attr('required_fields');
				$required_fields = Array($old_create_type => $required_fields);

				$asset->setAttrValue('required_fields', $required_fields);
				if (!$asset->saveAttributes()) {
					trigger_localised_error('CMS0026', sprintf(translate('Asset Builder Upgrade [from %s] FAILED'), '0.2'), E_USER_WARNING);
					return FALSE;
				}
				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'attributes');
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
			}
			$this->_upgrade03To04();
			pre_echo('ASSET BUILDER UPGRADE COMPLETE - VERSION 0.2');
		}//end if version 0.2 upgrade

		if ($current_version == '0.3') {
			pre_echo('STARTING ASSET BUILDER UPGRADE - VERSION 0.3');
			$this->_upgrade03To04();
			pre_echo('ASSET BUILDER UPGRADE COMPLETE - VERSION 0.3');
		}
		return TRUE;

	}//end _upgrade()


	/**
	* Upgrade from version 03 to 04
	*
	* @return boolean
	* @access public
	*/
	function _upgrade03To04()
	{
			// The create locations regime was radically overhauled between versions 0.3 and 0.4
			// to add more functions and more flexibility

			$root_folder = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_folder');
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($root_folder->id, $this->getAssetType(), FALSE);
			foreach ($children as $assetid => $type) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type);
				if (is_null($asset)) continue;


				if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'attributes')) {
					trigger_localised_error('CMS0025', sprintf(translate('Asset Builder Upgrade [from %1$s] FAILED - Unable to Acquire Lock on "%2$s" (#%3$s)', '0.3'), $asset->name, $asset->id), E_USER_WARNING);
					return FALSE;
				}

				$old_link_type = $asset->attr('create_link_type');

				// convert the fixed create locations by adding the link types
				$old_create_locations = $asset->attr('create_location');
				if (!Array($old_create_locations)) {
					$old_create_locations = Array(0 => $old_create_locations);
				}
				$fixed_create_locations = Array();
				foreach ($old_create_locations as $assetid => $null) {
					$fixed_create_locations[$assetid] = $old_link_type;
				}
				$asset->setAttrValue('create_location', $fixed_create_locations);

				// convert the selectable (optional) create locations by creating a set of the old ones
				if ($asset->attr('create_location_optional')) {
					$old_input_style = ($asset->attr('create_location_type') == 'select') ? 'dropdown' : 'radio';
					$old_selectable_locations = array_values($asset->attr('create_location_optional'));
					if (!is_array($old_selectable_locations)) {
						$old_selectable_locations = Array($old_selectable_locations);
					}
					$old_allow_multiple = $asset->attr('create_location_multiple') ? 1 : 0;
					$selectable_locations_set_details = Array();
					$location_details = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($old_selectable_locations);
					$selectable_locations_set_details['locations'] = Array();
					foreach ($old_selectable_locations as $assetid) {
						$selectable_locations_set_details['locations'][$assetid] = $location_details[$assetid]['name'];
					}
					echo "\n\n Allow Multiple is $old_allow_multiple\n\n";
					$selectable_locations_set_details['allow_multiple'] = $old_allow_multiple;
					$selectable_locations_set_details['allow_empty'] = 0;
					$selectable_locations_set_details['link_type'] = $old_link_type;
					$selectable_locations_set_details['input_style'] = $old_input_style;
					$asset->setAttrValue('create_location_optional', Array('First Selectable Location' =>  $selectable_locations_set_details));
				}

				// convert the create location rules into the new format
				$old_rules = $asset->attr('create_location_rules');
				$create_location_rules = Array();

				foreach ($old_rules as $create_type => $type_rules) {
					$create_location_rules[$create_type] = Array();
					foreach ($type_rules as $i => $rule_details) {
						$new_rule = Array(
										'attributes'	=> Array(),
										'comparators'	=> Array(),
										'values'		=> Array(),
										'locations'		=> Array(),
									);
						foreach ($rule_details as $attr_name => $conditions) {
							if ($attr_name == '_create_locations_') continue;
							foreach ($conditions as $comparator => $value) {
								$new_rule['attributes'][] = $attr_name;
								$new_rule['comparators'][] = $comparator;
								$new_rule['values'][] = $value;
							}
						}
						if (is_array($rule_details['_create_locations_'])) {
							foreach ($rule_details['_create_locations_'] as $assetid) {
								$new_rule['locations'][] = $assetid;
								$new_rule['link_types'][] = $old_link_type;
							}
						} else if ($rule_details['_create_locations_']) {
							$new_rule['locations'][$rule_details['_create_locations_']] = $old_link_type;
						}
						$create_location_rules[$create_type][] = $new_rule;
					}
				}
				$asset->setAttrValue('create_location_rules', $create_location_rules);

				if (!$asset->saveAttributes()) {
					trigger_localised_error('CMS0027', sprintf(translate('Asset Builder Upgrade [from %s] FAILED - Couldn\'t save attributes'), '0.3'), E_USER_WARNING);
					return FALSE;
				}

				// change the %create_location% keyword in bodycopies to the keyword for our new set
				$content_assetids = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, 'content_type_wysiwyg');
				foreach (array_keys($content_assetids) as $assetid) {
					$content_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($content_asset->id, 'attributes')) {
						trigger_localised_error('CMS0025', sprintf(translate('Asset Builder Upgrade [from %1$s] FAILED - Unable to Acquire Lock on "%2$s" (#%3$s)', '0.3'), $content_asset->name, $content_asset->id), E_USER_WARNING);
						return FALSE;
					}
					$html = $content_asset->attr('html');
					$html = str_replace('%create_location%', '%first_selectable_location_field%', $html);
					$content_asset->setAttrValue('html', $html);
					$content_asset->saveAttributes();
					$content_asset->_updated();
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($content_asset->id, 'attributes');
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($content_asset);
				}

				// regenerate the content files of the bodycopy containers so that they notice
				$container_assetids = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, 'bodycopy_container', FALSE);
				foreach (array_keys($container_assetids) as $assetid) {
					$container_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($container_asset->id, 'attributes')) {
						trigger_localised_error('CMS0025', sprintf(translate('Asset Builder Upgrade [from %1$s] FAILED - Unable to Acquire Lock on "%2$s" (#%3$s)', '0.3'), $container_asset->name, $container_asset->id), E_USER_WARNING);
						return FALSE;
					}
					$container_asset->saveSystemVersion();
					$GLOBALS['SQ_SYSTEM']->am->releaseLock($container_asset->id, 'attributes');
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($container_asset);
				}

				$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'attributes');
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

				if (!$asset->saveAttributes()) {
					trigger_localised_error('CMS0027', sprintf(translate('Asset Builder Upgrade [from %s] FAILED - Couldn\'t save attributes'), '0.3'), E_USER_WARNING);
					return FALSE;
				}

				$asset->_updated();
			}//end foreach

			// get rid of the old attributes
			$this->_deleteAttribute('create_location_multiple');
			$this->_deleteAttribute('create_location_type');
			$this->_deleteAttribute('create_link_type');

	}//end _upgrade03To04()


}//end class

?>
