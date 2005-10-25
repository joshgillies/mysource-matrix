<?php



require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Bulkmail_Post_Office_Management
*
* @see Asset_Management
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Post_Office_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Bulkmail_Post_Office_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'				=> Array(
												'added'		=> '0.1',
												'type'		=> 'text',
												'default'	=> '',
											   ),
						'groups'			=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'The groups to send this bulkmail to. All users in each group will become recipients',
											   ),
						'content_assetid'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'assetid',
												'default'		=> '',
												'description'	=> 'The asset content which will be sent in the bulkmail (ie, a standard page or similar)',
											   ),
						'content_details'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'User/Group based content generation and related details',
											   ),
						'server_details'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Details about which email server the bulkmails should be sent to',
											   ),
						'header_details'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'The header information required by the PEAR send() function',
											   ),
						'preview_log'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Log of the last 5 preview records',
											   ),
						'threshold'		=> Array(
												'added'			=> '0.1',
												'type'			=> 'serialise',
												'default'		=> Array('type' => 'ASAP', 'value' => ''),
												'description'	=> 'Whether to spread out the sending over x amount of time',
											   ),
					   );

	}//end constructor

}//end class
?>