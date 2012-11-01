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
* $Id: page_remote_content_management.inc,v 1.27.2.1 2012/11/01 06:49:02 cupreti Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Page_Remote_Content_Management
*
* @see Asset_Management
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.27.2.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Remote_Content_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Page_Remote_Content_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'remote_url'			=> Array(
													'added'		=> '0.1',
													'type'		=> 'text',
													'default'	=> 'http://www.squizsuite.net',
													'is_admin'	=> 0
												   ),
						'load_links'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array('allow_empty'	=> FALSE),
												   ),
						'verify_ssl'			=> Array(
													'added'			=> '1.7',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array('allow_empty'	=> FALSE),
												   ),
						'allow_redirect'		=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array('allow_empty'	=> FALSE),
												   ),
						'username'				=> Array(
													'added'		=> '0.1',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'password'				=> Array(
													'added'		=> '0.1',
													'type'		=> 'password',
													'default'	=> '',
												   ),
						'include_tags'			=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array('allow_empty'	=> FALSE),
												   ),
						'start_tag'				=> Array(
													'added'		=> '0.1',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'end_tag'				=> Array(
													'added'		=> '0.1',
													'type'		=> 'text',
													'default'	=> '',
												   ),
						'multi_occurence'		=> Array (
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array('allow_empty'	=> FALSE),
												   ),
						'no_tag_clear'			=> Array(
													'added'			=> '1.6',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array('allow_empty'	=> FALSE),
												   ),
						'timeout'				=> Array(
													'added'			=> '0.1',
													'type'			=> 'int',
													'default'		=> '30',
													'parameters'	=> Array('allow_negative' => FALSE),
												   ),
						'error_message'			=> Array(
													'added'		=> '0.1',
													'type'		=> 'wysiwyg',
													'default'	=> '<p>Content cannot be accessed</p>',
													'is_contextable'	=> TRUE,
												   ),
						'display_error'			=> Array(
													'added'			=> '0.1',
													'type'			=> 'boolean',
													'default'		=> TRUE,
													'parameters'	=> Array('allow_empty'	=> FALSE),
												   ),
						'binary_message'		=> Array(
													'added'		=> '0.1',
													'type'		=> 'wysiwyg',
													'default'	=> "<p>
																	The requested document appears to be a binary file.
																	Please click the url to open it in another window.<br />
																	<a href='%remote_url%' target='_blank'>%remote_url%</a>
																	</p>",
													'is_contextable'	=> TRUE,
												   ),
						'strip_images'			=> Array(
													'added'			=> '1.4',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array('allow_empty' => FALSE),
												   ),
						'tunnel_images'			=> Array(
													'added'			=> '1.4',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array('allow_empty' => FALSE),
												   ),
						'tunnel_media'			=> Array(
													'added'			=> '1.4',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array('allow_empty' => FALSE),
												   ),
						'tunnel_scripts'		=> Array(
													'added'			=> '1.4',
													'type'			=> 'boolean',
													'default'		=> FALSE,
													'parameters'	=> Array('allow_empty' => FALSE),
												   ),
						'post_variables'		=> Array(
													'added'		=> '1.5',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'tag_replacements'		=> Array(
													'added'		=> '1.6',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'url_white_list'		=> Array(
													'added'		=> '1.5',
													'type'		=> 'serialise',
													'default'	=> Array(),
												   ),
						'url_rewriting_options'	=> Array(
													'added'		=> '1.7',
													'type'		=> 'serialise',
													'default'	=> Array(
																	'all'			=> FALSE,
																	'remote_url'	=> FALSE,
																	'remote_domain'	=> TRUE,
																	'white_list'	=> FALSE,
																   ),
												   ),
					  );

	}//end constructor


	/**
	* Upgrade this asset to the most current version
	*
	* @param string	$current_version	The version this asset should be upgraded to
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		if (version_compare($current_version, '1.7','<')) {
			pre_echo('UPGRADING REMOTE CONTENT FROM v'.$current_version.' to v1.7');
			$ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('page_remote_content');
			foreach ($ids as $id) {
				$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
				$white_list = $asset->attr('url_white_list');
				$options = Array(
							'all'			=> FALSE,
							'remote_url'	=> FALSE,
							'remote_domain'	=> FALSE,
							'white_list'	=> FALSE,
						   );
				if ($asset->attr('only_local_links')) {
					if (empty($white_list)) {
						// restrict to local domain
						$options['remote_domain'] = TRUE;
					} else {
						// white list URLs plus local domain
						$options['remote_domain'] = TRUE;
						$options['white_list'] = TRUE;
					}
				} else {
					if (empty($white_list)) {
						$options['all'] = TRUE;
					} else {
						$options['white_list'] = TRUE;
						$options['remote_url'] = TRUE;
					}
				}
				$asset->setAttrValue('url_rewriting_options', $options);
				$asset->saveAttributes();
			}
			pre_echo('UPGRADE COMPLETE - '.count($ids).' REMOTE CONTENT PAGES UPGRADED');
		}//end if version matches

		return TRUE;

	}//end _upgrade()


}//end class
?>
