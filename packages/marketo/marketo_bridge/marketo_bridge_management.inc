<?php 
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';
require_once SQ_ATTRIBUTES_PATH.'/http_request/http_request.inc';

/**
* Oauth2_Token_Management
*
* @see Asset_Management
*
* @author  Edison Wang <ewang@squiz.com.au>
* @package MySource_Matrix_Packages
* @subpackage marketo
*/
class Marketo_Bridge_Management extends Asset_Management
{
	/**
	* Constructor
	*
	*/
	function __construct(&$pm)
	{
		$this->Asset_Management($pm);
		$this->vars = Array(
						'soap_endpoint'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'			=> '',
										),
		    				'soap_user_id'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'			=> '',
										),
		    				'soap_encryption_key'			=> Array(
											'added'			=> '0.1',
											'type'			=> 'text',
											'default'			=> '',
										),
						'debug'				=> Array(
											'added'		=> '0.1',
											'type'		=> 'boolean',
											'default'		=> FALSE,
											'parameters'    => Array(
													'allow_empty'   => FALSE,
											    ),
										),
						'get_lead'				=> Array(
											    'added'			=> '0.6',
											    'type'			=> 'boolean',
											    'default'		=> FALSE,
											    'parameters'	=> Array(
																    'allow_empty'=> FALSE,
															       ),
										),
		    		    		'get_lead_allowed_attributes'		=> Array(
											    'added'			=> '0.1',
											    'type'			=> 'text',
											    'default'			=> '',
										),
		    				'sync_lead'				=> Array(
											    'added'			=> '0.6',
											    'type'			=> 'boolean',
											    'default'		=> TRUE,
											    'parameters'	=> Array(
																    'allow_empty'=> FALSE,
															       ),
										),
		    		    		'sync_lead_allowed_attributes'		=> Array(
											    'added'			=> '0.1',
											    'type'			=> 'text',
											    'default'			=> '',
										 ),
						'email_trust_policy'			=> Array(
											    'added'			=> '0.1',
											    'type'			=> 'selection',
											    'default'		=> 'verified',
											    'parameters'	=> Array(
																    'multiple'		=> FALSE,
																    'allow_empty'   => FALSE,
																    'options'       => Array(
																					    'verified'	=> 'Verified Email or Trusted Email Source',
																					    'any' => 'Any Email passed to API',
														    ),
												    ),
										  ),
		    		    		 'sync_lead_email_source'		=> Array(
											    'added'			=> '0.1',
											    'type'			=> 'text',
											    'default'			=> '',
										 ),
		    		    		'get_lead_email_source'		=> Array(
											    'added'			=> '0.1',
											    'type'			=> 'text',
											    'default'			=> '',
										 ),
		    		    		 'get_lead_caching'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'			=> '-1',
												   ),
		    		    		 'munchkin_account_id'		=> Array(
											    'added'			=> '0.1',
											    'type'			=> 'text',
											    'default'			=> '',
										 ),
		    		    		 'munchkin_email_source'		=> Array(
											    'added'			=> '0.1',
											    'type'			=> 'text',
											    'default'			=> '',
										 ),
		    		    		 'munchkin_private_key'		=> Array(
											    'added'			=> '0.1',
											    'type'			=> 'text',
											    'default'			=> '',
										 ),
		    		    		 'munchkin_init_code'		=> Array(
											    'added'			=> '0.1',
											    'type'			=> 'text',
											    'default'			=> '<script type="text/javascript">
document.write(unescape("%3Cscript src=\'//munchkin.marketo.net/munchkin.js\' type=\'text/javascript\'%3E%3C/script%3E"));
</script>
<script>Munchkin.init(\'%munchkin_account_id%\');</script>',
										 ),
    		    									
			          );

	}//end constructor

}//end class
?>
