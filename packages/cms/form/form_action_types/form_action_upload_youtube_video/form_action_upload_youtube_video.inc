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
* $Id: form_action_upload_youtube_video.inc,v 1.6 2012/08/30 00:58:08 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/form/form_action/form_action.inc';
require_once SQ_ATTRIBUTES_PATH.'/oauth/oauth.inc';

/**
* Form_Action_Upload_Youtube_Video
*
* Purpose
*
*
* @author  Mohamed Haidar <mhaidar@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/


class Form_Action_Upload_Youtube_Video extends Form_Action
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint inline interface
	*
	*
	*/
	public static function paintInlineInterface(Form $form, $settings, Backend_Outputter $o, $prefix)
	{

		$settings['consumer_key']    = array_get_index($settings, 'consumer_key');
		$settings['consumer_secret'] = array_get_index($settings, 'consumer_secret');
		$settings['request_token']   = array_get_index($settings, 'request_token');
		$settings['request_token_secret'] = array_get_index($settings, 'request_token_secret');
		$settings['access_token']    = array_get_index($settings, 'access_token');
		$settings['access_token_secret'] = array_get_index($settings, 'access_token_secret');
		$settings['developer_key'] = array_get_index($settings, 'developer_key');
		$settings['privacy'] = array_get_index($settings, 'privacy', 'public');
		$settings['title'] = array_get_index($settings, 'title');
		$settings['title_question_id'] = array_get_index($settings, 'title_question_id', 0);
		$settings['description'] = array_get_index($settings, 'description');
		$settings['description_question_id'] = array_get_index($settings, 'description_question_id', 0);
		$settings['category'] = array_get_index($settings, 'category');
		$settings['category_question_id'] = array_get_index($settings, 'category_question_id', 0);
		$settings['video_question_id'] = array_get_index($settings, 'video_question_id', 0);
		$settings['delete_video_file'] = array_get_index($settings, 'delete_video_file', 1);
		$settings['upload_timeout'] = array_get_index($settings, 'upload_timeout', 300);

		$consumer_details_locked = (!empty($settings['request_token'])) ? TRUE : FALSE;

		ob_start();

		if (!empty($_REQUEST['oauth_verifier'])) hidden_field('oauth_verifier', $_REQUEST['oauth_verifier']);

		?>
		<div style="padding: 5px">
		<p class="sq-backend-section-subheading"><?php echo translate('Client Credentials') ?></p>


		<table class="sq-backend-table">
			<col width="20%" />
			<col width="80%" />
			<tr>
				<th><?php echo translate('Consumer key:') ?></th>

				<td><?php
				if (($consumer_details_locked === FALSE)) {
					text_box($prefix.'[consumer_key]', $settings['consumer_key'], 50);
				} else {
					echo $settings['consumer_key'];
					hidden_field($prefix.'[consumer_key]', $settings['consumer_key']);
				}
				?></td>
			</tr>
			<tr>
				<th><?php echo translate('Consumer secret string:') ?></th>

				<td><?php
				if (($consumer_details_locked === FALSE)) {
					text_box($prefix.'[consumer_secret]', $settings['consumer_secret'], 50);
				} else {
					echo $settings['consumer_secret'];
					hidden_field($prefix.'[consumer_secret]', $settings['consumer_secret']);
				}
				?></td>
			</tr>
			<tr>
				<th><?php echo translate('Developer key:') ?></th>

				<td><?php
					text_box($prefix.'[developer_key]', $settings['developer_key'], 50);
				?></td>
			</tr>
			<tr>
				<th><?php echo translate('Authentication:') ?></th>

				<td><?php
				if ((empty($settings['consumer_key']) === TRUE) || (empty($settings['consumer_secret']) === TRUE)) {
					echo translate('<strong style="color: #f00">Client credentials missing.</strong>');

				} else {
					if (!empty($settings['access_token'])){
						echo translate('<strong style="color: #060">Access Token Granted</strong>');

					} else if (empty($settings['access_token']) && !empty($_REQUEST['oauth_verifier'])){
						$value = translate('Get Access Token');

						submit_button($prefix.'[get_access_token]', $value, 'main_form.submit()');
						if (!empty($settings['access_token_errors'])){
							?><br /><strong style="color: #f00">Error: failed to gain access token</strong><?php
							foreach ($settings['access_token_errors'] as $error) {
								?><br/><?php echo $error ?><?php
							}
						}
					} else if (!empty($settings['request_token'])){
							$callback_url = self::_getCallbackURL($prefix);
	                        $url = 'https://www.google.com/accounts/OAuthAuthorizeToken?oauth_token='.$settings['request_token'].'&oauth_callback='.urlencode($callback_url);
	                        echo '<a target="_blank" href="'.$url.'">'.translate('Authorise Token').'</a>';

					} else {
						$value = translate('Get Request Token');

						submit_button($prefix.'[get_request_token]', $value, 'main_form.submit()');
						if (!empty($settings['request_token_errors'])){
							?><br /><strong style="color: #f00">Error: failed to gain request token</strong><?php
							foreach ($settings['request_token_errors'] as $error) {
								?><br/><?php echo $error ?><?php
							}
						}
					}
				}
				?></td>
			</tr>
			<tr>
				<th><?php echo translate('Reset Authentication?') ?></th>

				<td><?php
				check_box($prefix.'[deauthorise]');
				?></td>
			</tr>
		</table>
		</div>
		<div style="padding: 5px">
		<?php echo translate('<p>You need to have a YouTube account linked with a Google account. To obtain a Consumer Key and Secret, please see the <a target="_blank" href="http://code.google.com/apis/accounts/docs/RegistrationForWebAppsAuto.html#new">Registration for Web-Based Applications</a> page for an explanation of the registration process.</p>
    <p><strong>Note:</strong> Leave the certificate field blank when completing your registration. Google will generate an OAuth consumer secret value, which will display on your domain\'s registration page after you have completed the registration process.</p>
    <p>To obtain a Developer Key please visit your <a  target="_blank" href="http://code.google.com/apis/youtube/dashboard/">Account Dashboard</a> page.</p>
    <p>Once you have entered your credentials, there are 3 steps to get authenticated:
    <ol>
    <li>Click the "Get Request Token" button</li>
    <li>Click the "Authorise Token" link which takes you to your account where you need log in and approve the token. You will automatically be redirected back here.</li>
    <li>Immediately click the "Get Access Token" button.</li>
    </ol>
    If there is an error at any stage or you wish to change accounts, Reset the Authentication.</p>') ?>

		</div>

        <div style="padding: 5px">
		<p class="sq-backend-section-subheading"><?php echo translate('YouTube Video Setup') ?></p>


		<table class="sq-backend-table">
			<tr>
				<th>&nbsp;</th>
				<th>Video Settings</th>
				<th>Form Question Id</th>
			</tr>
			<tr>
				<th><?php echo translate('Privacy:') ?></th>

				<td><?php
				$options = Array(
							'public'	=> 'Public',
							'private'	=> 'Private',
							'unlisted'	=> 'Unlisted'
						   );
				combo_box($prefix.'[privacy]', $options, FALSE, $settings['privacy']);

				?></td>
				<td>&nbsp;</td>
			</tr>
			<tr>
				<th><?php echo translate('Title:') ?></th>

				<td><?php
					text_box($prefix.'[title]', $settings['title'], 50, 100);
				?> </td>
				<td><?php
					$type_codes = Array (
									  'form_question_type_text' => 'I',
									);
					asset_finder($prefix.'[title_question_id]', $settings['title_question_id'], $type_codes);
				?></td>
			</tr>
			<tr>
				<th><?php echo translate('Description:') ?></th>

				<td><?php
					text_area($prefix.'[description]', $settings['description'], 46, 10, 5000);
				?> </td>
				<td><?php
					$type_codes = Array (
									  'form_question_type_text' => 'I',
									);
					asset_finder($prefix.'[description_question_id]', $settings['description_question_id'], $type_codes);
				?></td>
			</tr>
			<tr>
				<th><?php echo translate('Category:') ?></th>

				<td><?php
					text_box($prefix.'[category]', $settings['category'], 50);
				?> </td>
				<td><?php
					$type_codes = Array (
									  'form_question_type_text' => 'I',
									  'form_question_type_select' => 'I',
									);
					asset_finder($prefix.'[category_question_id]', $settings['category_question_id'], $type_codes);
				?></td>
			</tr>
			<tr>
				<th><?php echo translate('Video Source:') ?></th>

				<td>&nbsp;</td>
				<td><?php
					$type_codes = Array (
									  'form_question_type_file_upload' => 'I',
									);
					asset_finder($prefix.'[video_question_id]', $settings['video_question_id'], $type_codes);
				?></td>
			</tr>
			<tr>
				<th><?php echo translate('Delete Video from server?') ?></th>

				<td><?php
				$options = Array(
							1	=> 'Yes',
							0	=> 'No'
						   );
				combo_box($prefix.'[delete_video_file]', $options, FALSE, $settings['delete_video_file']);
				?></td>
				<td>&nbsp;</td>
			</tr>
			<tr>
				<th><?php echo translate('Video Upload Timeout (sec)') ?></th>

				<td><?php
					int_text_box($prefix.'[upload_timeout]', $settings['upload_timeout'], FALSE);
				?></td>
				<td>&nbsp;</td>
			</tr>
		</table>
		</div>
		<div style="padding: 5px">
		<?php echo translate('<p>For the Title, Description and Category, you may set a default value and/or select a Form Question asset to dynamically populate the value from its submission answer. The Form Question answer takes priority over the default value. Category is compulsory. Description is optional. A missing Title will be populated with the video file name.</p>
    <p>The Video Source field is compulsory and must be a File Upload Form Question.</p>
    <p>Video files are only deleted from the server if the File Upload Form Question does not specify a "Create Location".</p>') ?>

		</div>

        <?php

		$contents = ob_get_clean();

		echo $contents;

	}//end paintInlineInterface()


	/**
	* Process inline interface
	*
	*
	*/
	public static function processInlineInterface(Form $form, &$settings, Backend_Outputter $o, $prefix)
	{
		$settings['before_submit'] = TRUE;

        $request_data = $_REQUEST[$prefix];
		$deauthorise = (boolean)array_get_index($request_data, 'deauthorise', FALSE);

		if ($deauthorise === TRUE) {
			// Blank out current tokens
			$settings['request_token'] = NULL;
			$settings['request_token_secret'] = NULL;
			$settings['request_token_errors'] = NULL;

			$settings['access_token'] = NULL;
			$settings['access_token_secret'] = NULL;
			$settings['access_token_errors'] = NULL;

			unset($_REQUEST['oauth_verifier']);

			return TRUE;
		}

		$settings['consumer_key'] = array_get_index($request_data, 'consumer_key');
		$settings['consumer_secret'] = array_get_index($request_data, 'consumer_secret');
		$settings['developer_key'] = array_get_index($request_data, 'developer_key');
		$settings['request_token_errors'] = NULL;
		$settings['access_token_errors'] = NULL;

		if (isset($request_data['get_request_token'])) {
			self::processRequestToken($settings ,$prefix);
		} else if (isset($request_data['get_access_token'])) {
			self::processAccessToken($settings, $_REQUEST['oauth_verifier']);
		}

		$settings['privacy'] = array_get_index($request_data, 'privacy', 'public');
		$settings['title'] = array_get_index($request_data, 'title');
		$settings['title_question_id'] = array_get_index(array_get_index($request_data, 'title_question_id', Array()), 'assetid', 0);
		$settings['description'] = array_get_index($request_data, 'description');
		$settings['description_question_id'] = array_get_index(array_get_index($request_data, 'description_question_id', Array()), 'assetid', 0);
		$settings['category'] = array_get_index($request_data, 'category');
		$settings['category_question_id'] = array_get_index(array_get_index($request_data, 'category_question_id', Array()), 'assetid', 0);
		$settings['video_question_id'] = array_get_index(array_get_index($request_data, 'video_question_id', Array()), 'assetid', 0);
		$settings['delete_video_file'] = array_get_index($request_data, 'delete_video_file', 1);
		$settings['upload_timeout'] = (!empty($request_data['upload_timeout'])) ? $request_data['upload_timeout'] : 300;

        return TRUE;

	}//end processInlineInterface()


	/**
	* Get Callback URL
	*
	* @param string $prefix
	*
	* @return string
	* @access private
	*/
    private function _getCallbackURL($prefix){

    	$callback_url = replace_query_string_vars(Array(), NULL, NULL, TRUE);
        $additional_params = 'SQ_BACKEND_PAGE=frames';
        $action_id = array_get_index($_REQUEST, $prefix.'_new_selected_action', '');
        if (strlen($action_id) == 0) $action_id = array_get_index($_REQUEST, $prefix.'_selected_action', '');
        $additional_params .= '&'.$prefix.'_new_selected_action='.$action_id;
        if (strpos($callback_url, '?') === FALSE) {
            $callback_url .= '?'.$additional_params;
        } else {
            $callback_url .= '&'.$additional_params;
        }

		return $callback_url;

    }//end _getCallbackURL()


    /**
	* Process Request Token
	*
	* @param array	&$settings
	* @param string	$prefix
	*
	* @return void
	* @access public
	*/
	public static function processRequestToken(&$settings, $prefix)
	{

        $callback_url = self::_getCallbackURL($prefix);

		$oauth_attr = new Asset_Attribute_OAuth();
		$oauth_attr_values = Array(
								'consumer_key'		=> $settings['consumer_key'],
								'consumer_secret'	=> $settings['consumer_secret'],
								'request_token_url'	=> 'https://www.google.com/accounts/OAuthGetRequestToken?scope=http://gdata.youtube.com',
								'method'			=> 'GET',
                                'callback_url'      => $callback_url,
								'signature_method'	=> 'HMAC-SHA1',
								'request_headers'	=> Array(),
								'request_body'		=> '',
								'timeout'			=> 30,
								'cache_options'		=> 'NEVER',
								'follow_redirect'	=> FALSE,
							 );
		$oauth_attr->setValue($oauth_attr_values);

		$settings['request_token'] = NULL;
		$settings['request_token_secret'] = NULL;
		$settings['request_token_errors'] = NULL;

		$settings['access_token'] = NULL;
		$settings['access_token_secret'] = NULL;
		$settings['access_token_errors'] = NULL;

		$result = $oauth_attr->getRequestToken();

		if (!isset($result['request_token']['oauth_token']) || !isset($result['request_token']['oauth_token_secret'])) {
			$settings['request_token_errors'] = $oauth_attr->getErrors();
		} else {
			$settings['request_token'] = $result['request_token']['oauth_token'];
			$settings['request_token_secret'] = $result['request_token']['oauth_token_secret'];
		}

	}//end processRequestToken()


    /**
	* Process Access Token
	*
	* @param array	&$settings
	* @param string	$oauth_verifier
	*
	* @return void
	* @access public
	*/
	public static function processAccessToken(&$settings, $oauth_verifier)
	{

		$oauth_attr = new Asset_Attribute_OAuth();
		$oauth_attr_values = Array(
								'consumer_key'		=> $settings['consumer_key'],
                                'consumer_secret'   => $settings['consumer_secret'],
								'access_token_url'	=> 'https://www.google.com/accounts/OAuthGetAccessToken',
								'method'			=> 'POST',
                                'callback_url'      => '',
								'signature_method'	=> 'HMAC-SHA1',
								'request_headers'	=> Array(),
								'request_body'		=> '',
								'timeout'			=> 30,
								'cache_options'		=> 'NEVER',
								'follow_redirect'	=> FALSE,
							 );
		$oauth_attr->setValue($oauth_attr_values);

		// Blank out current request token settings
		$settings['access_token'] = NULL;
		$settings['access_token_secret'] = NULL;
		$settings['access_token_errors'] = NULL;

		// Grab a new request token
		$result = $oauth_attr->getAccessToken(urldecode($settings['request_token']), $oauth_verifier, urldecode($settings['request_token_secret']));

		if (!isset($result['access_token']['oauth_token']) || !isset($result['access_token']['oauth_token_secret'])) {
			$settings['access_token_errors'] = $oauth_attr->getErrors();
		} else {
			$settings['access_token'] = $result['access_token']['oauth_token'];
			$settings['access_token_secret'] = $result['access_token']['oauth_token_secret'];
		}

	}//end processAccessToken()


	/**
	* Paint summary description
	*
	*
	*/
	public static function paintSummary(Form $form, $settings, Backend_Outputter $o, $prefix)
	{
		?><table class="no-borders">
			<colgroup>
				<col width="80" />
				<col/>
			</colgroup>
			<tbody>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>Authentication:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($settings['access_token'])) {
						echo translate('<strong style="color: #060">Access Token Granted</strong>');

					} else {
						?><span class="sq-backend-warning">No Access Token.</span><?php
					}
					?></p></td>
				</tr>
				<tr>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><strong>Video Source:</strong></p></td>
					<td class="sq-backend-table-cell" style="vertical-align: top"><p><?php
					if (!empty($settings['video_question_id'])) {
						echo get_asset_tag_line($settings['video_question_id']);
					} else {
						?><span class="sq-backend-warning">No video source specified.</span><?php
					}
					?></p></td>
				</tr>
			</tbody>
		</table>
		<?php

	}//end paintSummary()


	/**
	* Execute form action
	*
	*/
	public static function execute(Form $form, $settings)
	{

		$submission = (isset($form->submission_asset)) ? $form->submission_asset->attr('attributes') : NULL;
		if (empty($submission)){
			trigger_error("Unable to extract submission from ".$form->type()." (Id: #$form->id)", E_USER_WARNING);
			$form->submission_errors[] = 'Failed to upload video file to Youtube';
			return FALSE;
		}

		if (!empty($settings['video_question_id'])){
			if (!empty($submission['answers'][$settings['video_question_id']]['extra_data']['filesystem_path'])){
				$video_path = $submission['answers'][$settings['video_question_id']]['extra_data']['filesystem_path'];
				$temp_path = FALSE;
			} else if (!empty($submission['answers'][$settings['video_question_id']]['extra_data']['temp_filesystem_path'])){
				$video_path = $submission['answers'][$settings['video_question_id']]['extra_data']['temp_filesystem_path'];
				$temp_path = TRUE; //the case when using create locations.
			}
		}

		if (empty($video_path)){
			trigger_error("Unable to locate video source from ".$form->type()." (Id: #$form->id)", E_USER_WARNING);
			$form->submission_errors[] = 'Failed to upload video file to Youtube';
			return FALSE;
		}

		if (!empty($settings['category_question_id']) && !empty($submission['answers'][$settings['category_question_id']]['answer'])){
			$settings['category'] = $submission['answers'][$settings['category_question_id']]['answer'];
		}
		if (!empty($settings['title_question_id']) && !empty($submission['answers'][$settings['title_question_id']]['answer'])){
			$settings['title'] = $submission['answers'][$settings['title_question_id']]['answer'];
		}

		if (empty($settings['title'])) $settings['title'] = basename($video_path);

		if (!empty($settings['description_question_id']) && !empty($submission['answers'][$settings['description_question_id']]['answer'])){
			$settings['description'] = $submission['answers'][$settings['description_question_id']]['answer'];
		}

		// convert relative path to absulute path
		if(strpos($video_path, '/') !== 0) {
		    $video_path = SQ_SYSTEM_ROOT.'/'.$video_path;
		}

		$keywords = Array();
		if (self::processUploadVideo($settings, $video_path, $keywords) === TRUE){
			$form->form_action_keywords = $keywords;
			if ($settings['delete_video_file'] == 1 && !$temp_path){
				if (unlink($video_path) == TRUE){
					$extra_data = $form->submission_asset->getExtraData($settings['video_question_id']);
					unset($extra_data['filesystem_path']);
					$form->submission_asset->setExtraData($settings['video_question_id'], $extra_data);
					$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
					$form->submission_asset->saveAttributes();
					$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				}
			}
		} else {
			trigger_error("Failed to upload video file to Youtube for ".$form->type()." (Id: #$form->id)", E_USER_WARNING);
			$form->submission_errors[] = 'Failed to upload video file to Youtube';
			return FALSE;
		}

		return TRUE;

	}//end execute()


	/**
	* Is this action valid?
	*
	*/
	public static function isValid(Form $form, $settings)
	{
		if (empty($settings['access_token'])) return FALSE;
		if (empty($settings['developer_key'])) return FALSE;
		if (empty($settings['video_question_id'])) return FALSE;
		if (empty($settings['category']) && empty($settings['category_question_id'])) return FALSE;

		return TRUE;

	}//end isValid()


	/**
	* Process Upload Video
	*
	* @param array	$settings
	* @param string $video_path
	* @param string &$keywords
	*
	* @return boolean
	* @access public
	*/
	public static function processUploadVideo($settings, $video_path, &$keywords)
	{
		$url = 'http://uploads.gdata.youtube.com/feeds/api/users/default/uploads';

		$xml_api = '<?xml version="1.0"?><entry xmlns="http://www.w3.org/2005/Atom" xmlns:media="http://search.yahoo.com/mrss/" xmlns:yt="http://gdata.youtube.com/schemas/2007">';
		if ($settings['privacy'] == 'unlisted') $xml_api .= '<yt:accessControl action=\'list\' permission=\'denied\' />';
		$xml_api .=	'<media:group><yt:incomplete/>';
		if ($settings['privacy'] == 'private') $xml_api .= '<yt:private/>';
		if (!empty($settings['title'])) $xml_api .= '<media:title type="plain">'.$settings['title'].'</media:title>';
		if (!empty($settings['description'])) $xml_api .= '<media:description type="plain">'.$settings['description'].'</media:description>';
		if (!empty($settings['category'])) $xml_api .= '<media:category scheme="http://gdata.youtube.com/schemas/2007/categories.cat">'.$settings['category'].'</media:category>';
		$xml_api .=	'</media:group></entry>';

		$oauth_attr = new Asset_Attribute_OAuth();
		$oauth_attr_values = Array(
								'consumer_key'		=> $settings['consumer_key'],
								'consumer_secret'	=> $settings['consumer_secret'],
								'method'			=> 'POST',
								'signature_method'	=> 'HMAC-SHA1',
								'request_headers'	=> '',
								'request_body'		=> '',
								'timeout'			=> 30,
								'cache_options'		=> 'NEVER',
								'follow_redirect'	=> FALSE,
							 );
		$oauth_attr->setValue($oauth_attr_values);

		$auth_header = $oauth_attr->getUserDataAuthHeader($url, $settings['access_token'], $settings['access_token_secret'], FALSE);
		$request = self::_HTTPUploadRequest($url, $xml_api, $video_path, $auth_header, $settings);

		if($request['status'] == 'ok'){
			try {
				$xml = new SimpleXMLElement($request['content']);
			}catch(Exception $e){
				trigger_error("Failed to parse Youtube API XML: ".$e->getMessage()."\n", E_USER_WARNING);
				return FALSE;
			}

			$video_id = (isset($xml->id)) ? (string)$xml->id : '';
			if ($video_id){
				$video_id = (strpos($video_id, 'video:') !== FALSE) ? substr($video_id, strpos($video_id, 'video:') + strlen('video:')) : $video_id;

				$published_date = (isset($xml->published)) ? substr((string)$xml->published, 0, strpos((string)$xml->published, 'T')) : '';
				$video_link = '';
				if (isset($xml->link)){
					foreach($xml->link as $link){
						$attr = $link->attributes();
						if (isset($attr['rel']) && $attr['rel'] == 'alternate'){
							$video_link = (string)$attr['href'];
							break;
						}
					}
				}

				$keywords = Array(
								'youtube_video_id' => $video_id,
								'youtube_video_published_date' => $published_date,
								'youtube_video_link'	=> $video_link
								);
				return TRUE;

			} else if (isset($xml->error)){
				foreach($xml->error as $error){
					$errors[] = (Array)$error;
				}
				trigger_error("Youtube API Error:\n".print_r($errors, TRUE)."\n", E_USER_WARNING);
				return FALSE;
			}

		} else if($request['status'] == 'err'){
			trigger_error('Youtube File Upload Fail: '.$request['error'], E_USER_WARNING);
			return FALSE;
		}

		return FALSE;

	}//end processUploadVideo()


	/**
	* HTTP Upload Request
	*
	* @param string $url
	* @param string $api_xml
	* @param string $video_path
	* @param string $auth_header
	* @param string $settings
	*
	* @return array
	* @access private
	*/
	private function _HTTPUploadRequest($url, $api_xml, $video_path, $auth_header, $settings) {

		$vide_h = fopen($video_path, "rb");
		if ($vide_h === FALSE) return Array(
							            'status' => 'err',
							            'error' => "Unable to open video file: $video_path"
							        	);

		$video_size = filesize($video_path);
		$video = fread($vide_h, $video_size);
		fclose($vide_h);

		$file_name = basename($video_path);

	    $url = parse_url($url);
	    $host = $url['host'];
	    $path = $url['path'];

	    $fp = fsockopen($host, 80, $errno, $errstr, $settings['upload_timeout']);

	    if ($fp){

	        $boundary_string = uniqid();
	    	$post_body = '';

	    	$post_body .= "--$boundary_string\r\n";
	        $post_body .= "Content-Type: application/atom+xml; charset=UTF-8\r\n\r\n";
	        $post_body .= $api_xml;
	 		$post_body .= "\r\n--$boundary_string\r\n";
	 		$post_body .= "Content-Type: application/octet-stream\r\n";
	 		$post_body .= "Content-Transfer-Encoding: binary\r\n\r\n";

	 		$post_end = "\r\n--$boundary_string--\r\n";

	 		$size = strlen($post_body) + $video_size + strlen($post_end);
	 		$post_body .= $video;
	 		$post_body .= $post_end;

	        // send the request headers:
	        $header = "POST $path HTTP/1.1\r\n";
	        $header .= "Host: $host\r\n";
	        $header .= $auth_header."\r\n";
	 		$header .= "GData-Version: 2\r\n";
	 		$header .= "X-GData-Key: key=".$settings['developer_key']."\r\n";
	 		$header .= "Slug: $file_name\r\n";
	        $header .= "Content-Type: multipart/related; boundary=$boundary_string\r\n";
	        $header .= "Content-Length: ". $size ."\r\n";
	        $header .= "Connection: close\r\n\r\n";

	        fwrite($fp, $header.$post_body);
	        $result = '';
	        while (!feof($fp)) {
        		$result .= fgets($fp);
    		}

	    } else {
	        return array(
	            'status' => 'err',
	            'error' => "$errstr ($errno)"
	        );
	    }

	    // close the socket connection:
	    fclose($fp);

	    // split the result header from the content
	    $result = explode("\r\n\r\n", $result, 2);
	    $header = isset($result[0]) ? $result[0] : '';
	    $content = isset($result[1]) ? $result[1] : '';

	    // return as structured array:
	    return array(
	        'status' => 'ok',
	        'header' => $header,
	        'content' => $content
	    );
	}//end _HTTPUploadRequest()

}//end class

?>
