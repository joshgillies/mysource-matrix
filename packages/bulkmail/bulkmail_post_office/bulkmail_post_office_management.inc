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
* $Id: bulkmail_post_office_management.inc,v 1.27 2012/08/30 00:57:13 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';


/**
* Bulkmail_Post_Office_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.27 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Post_Office_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		parent::__construct($pm);

		if (!strlen(SQ_CONF_DEFAULT_EMAIL)) {
			trigger_localised_error('SYS0315', translate('System Default email address has not been set.'), E_USER_WARNING)
		}

		$this->vars = Array(
						'name'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'recipients'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'The recipients to send this bulkmail to. All users/bulkmail_users in each group will become recipients',
											   ),
						'content_id'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'The asset we will be using as content in the email',
											   ),
						'content_gen'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'one_user',
												'description'	=> 'The method of content generation',
												'parameters'	=> Array(
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																	'options'		=> Array(
																						'each_user'	=> 'Generate as recipient',
																						'one_user'	=> 'Generate as set user',
																					   ),
																   ),
											   ),
						'content_gen_as'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'The user to generate the content with',
											   ),
						'content_design'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'The design to use for the content asset',
											   ),
						'content_layout'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'The layout to use for the content asset',
											   ),
						'server_details'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array('driver' => 'sendmail'),
												'description'	=> 'Details about which email server the bulkmails should be sent to',
											   ),
						'from'				=> Array(
												'added'			=> '0.1',
												'type'			=> 'email',
												'default'		=> SQ_CONF_DEFAULT_EMAIL,
												'description'	=> 'Sending address as per RFC822',
											   ),
						'subject'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'text',
												'default'		=> '',
												'description'	=> 'Subject field of the email',
											   ),
						'header_details'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Extra header detail fields',
											   ),
						'threshold'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Threshold rules that will be enforced by the Bulk Mailer when it runs',
											   ),
						'threshold_types'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(
																	// bulkmail_threshold_rule_excluded_time
																	0	=> Array(
																			'multiple'	=> TRUE,
																			'params'	=> Array(
																							'from'	=> 0,
																							'to'	=> 0,
																							'days'	=> Array(),
																						   ),
																		   ),
																	// bulkmail_threshold_rule_send_rate
																	1	=> Array(
																			'multiple'	=> FALSE,
																			'params'	=> Array(
																							'value'	=> 60,
																							'unit'	=> 60,
																						   ),
																		   ),
																   ),
												'description'	=> 'Threshold rule types and their default values',
											   ),
						'bulkmail_mode'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'selection',
												'default'		=> 'live',
												'parameters'	=> Array(
																	'multiple'		=> FALSE,
																	'allow_empty'	=> FALSE,
																	'options'		=> Array(
																						'live'	=> 'Live Mode',
																						'test'	=> 'Test Mode',
																					   ),
																   ),
											   ),
					  );

	}//end constructor



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
		parent::_upgrade($current_version);

		if (version_compare($current_version, '0.2', '<')) {
			echo('UPGRADING BULKMAIL POST OFFICE FROM '.$current_version."\n");
			// Version 0.2 ownwards the sendmail path and arguments are retrieved from External tools config (tools.inc)
			// rather than stored in attribute

			$count = 0;

			pre_echo('Updating Bulkmail Post Offices...');

			// Get all Search Page assets
			$bulkmail_post_offices = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('bulkmail_post_office', FALSE, TRUE);

			$sendmail_path = '';
			$sendmail_args = '';

			// Update assets
			foreach ($bulkmail_post_offices as $id => $type) {
				$post_office = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);

				$server_details = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('server_details', 'bulkmail_post_office', Array($post_office->id));
				$server_details = unserialize($server_details[$post_office->id]);
				if(isset($server_details['sendmail_path']) && !empty($server_details['sendmail_path']))
					$sendmail_path = $server_details['sendmail_path'];
				if(isset($server_details['sendmail_args']) && !empty($server_details['sendmail_args']))
					$sendmail_args = $server_details['sendmail_args'];

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($post_office, TRUE);
				unset($post_office);

				$count++;
			}//end foreach search pages


			$vars['SQ_TOOL_SENDMAIL_PATH'] = $sendmail_path;
			$vars['SQ_TOOL_SENDMAIL_ARGS'] = $sendmail_args;

			require_once SQ_INCLUDE_PATH.'/external_tools_config.inc';
			$cfg = new External_Tools_Config();
			if (!$cfg->save($vars, FALSE, TRUE)) {
			    echo "Sendmail path and arguments for bulkmail can not be saved\n";
			    return FALSE;
			}


			pre_echo('Updated - '.$count.' bulkmail post office(s)...');

			pre_echo('BULKMAIL POST OFFICE SUCCESSFULLY UPGRADED TO VERSION 0.2');
		}//end if
		return TRUE;

	}//end _upgrade()

}//end class
?>
