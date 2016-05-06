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
		$settings['oauth2_token_assetid'] = array_get_index($settings, 'oauth2_token_assetid', '');
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

		ob_start();
		?>
		<div>
		<p class="sq-backend-section-subheading"><?php echo translate('Client Credentials') ?></p>
		<table class="sq-backend-table">
			<col width="20%" />
			<col width="80%" />
			<tr class="sq-backend-row">
				<th class="sq-backend-field" style="vertical-align:middle"><?php echo translate('Oauth2 Token asset:') ?></th>
				<td class="sq-backend-data"><?php
					asset_finder($prefix.'[oauth2_token_assetid]', $settings['oauth2_token_assetid'], Array('oauth2_token' => 'I'));
				?></td>
			</tr>
		</table>
		</div>
		<div class="sq-backend-smallprint">
			<p>
			<?php
				echo translate('Select Matrix Oauth2 Token asset having valid OAuth 2.0 access token set for YouTube Data API v3.');
			?>
			</p>
			<p>
			<?php
				echo translate('You will need to have a YouTube account linked with a Google account. To obtain a Client ID and Clinet Secret, please see the <a target="_blank" href="https://developers.google.com/youtube/registering_an_application">Registration for Web-Based Applications</a> page for an explanation of the registration process.');
			?>
			</p>
		</div>

        <div style="padding: 5px">
		<p class="sq-backend-section-subheading"><?php echo translate('YouTube Video Setup') ?></p>
		<table class="sq-backend-table">
		<col width="20%" />
		<col width="40%" />
		<col width="40%" />
			<tr>
				<th style='min-width:200px;'>&nbsp;</th>
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
		<div class="sq-backend-smallprint">
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

		$settings['oauth2_token_assetid'] = array_get_index(array_get_index($request_data, 'oauth2_token_assetid', Array()), 'assetid', 0);
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
					if (self::_getAccessToken($settings)) {
						echo translate('<strong style="color: #060">Oauth2 token found.</strong>');

					} else {
						?><span class="sq-backend-warning">Oauth2 token not found.</span><?php
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
			$form->submission_errors[] = 'Failed to upload video file to YouTube';
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
			$form->submission_errors[] = 'Failed to upload video file to YouTube';
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
			trigger_error("Failed to upload video file to YouTube for ".$form->type()." (Id: #$form->id)", E_USER_WARNING);
			$form->submission_errors[] = 'Failed to upload video file to YouTube';
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
		if (empty($settings['video_question_id'])) return FALSE;
		if (empty($settings['category']) && empty($settings['category_question_id'])) return FALSE;
		if (!self::_getAccessToken($settings)) return FALSE;

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
		$access_token = self::_getAccessToken($settings);
		if (!$access_token) {
			return FALSE;
		}

		if (!is_file($video_path)) {
			return FALSE;
		}

		// Video properties
		$video_title = array_get_index($settings, 'title', '');
		$video_description = array_get_index($settings, 'description', '');
		$video_category_id = array_get_index($settings, 'category', '');
		$video_privacy_status = array_get_index($settings, 'privacy', 'private');

		$request_url = 'https://www.googleapis.com/upload/youtube/v3/videos?part=snippet,status&uploadType=resumable';
		$video_params = Array(
							'snippet' => Array(
											'title' => $video_title,
											'description' => $video_description,
											'categoryId' => $video_category_id,
											'tags' => Array(),
										),
							'status'  => Array(
											'privacyStatus' => $video_privacy_status
										),
						);
		$request_body = json_encode($video_params);
		$content_length = ini_get('mbstring.func_overload') ? mb_strlen($request_body , '8bit') : strlen($request_body);
		$file_size = filesize($video_path);

		$curl_opts = Array(
						'POST'			 => TRUE,
						'POSTFIELDS'	 => json_encode($video_params),
						'RETURNTRANSFER' => TRUE,
						'HTTPHEADER' 	 => Array(
											'Authorization: Bearer '.$access_token,
											'Content-type: application/json; charset=UTF-8',
											'Content-length: '.$content_length,
											'X-upload-content-type: video/*',
											'X-upload-content-length: '.$file_size,
										),
				);
		$response = fetch_url($request_url, $curl_opts, Array(), TRUE, TRUE);

		$response_body = array_get_index($response, 'response', '');
		$response_headers = array_get_index($response, 'response_headers', '');
		$response_code = isset($response['curlinfo']) && isset($response['curlinfo']['http_code']) ? $response['curlinfo']['http_code'] : '';

		if ($response_code == 200 && !empty($response_headers)) {
			// Get the file upload request URL from the response headers
			$location = '';
			foreach($response_headers as $header) {
				$header = trim($header);
				if (strpos($header, 'Location:') === 0) {
					$location = substr($header, 10);
					break;
				}
			}
			if (!empty($location) && strpos($location, 'https:') === 0) {
				// Now upload the video
				$file_content = file_get_contents($video_path);
				$content_length = ini_get('mbstring.func_overload') ? mb_strlen($file_content , '8bit') : strlen($file_content);

				$curl_opts = Array(
								'POST' 				=> TRUE,
								'POSTFIELDS'	 	=> $file_content,
								'RETURNTRANSFER' 	=> TRUE,
								'BINARYTRANSFER' 	=> TRUE,
								'TIMEOUT' 			=> $settings['upload_timeout'],
								'HTTPHEADER' 		=> Array(
														'Content-length: '.$content_length,
														'Content-type: application/json; charset=UTF-8',
													),
						);
				$response_body = fetch_url($location, $curl_opts, Array(), TRUE, TRUE);
				if (!empty($response_body['response'])) {
					$upload_info = @json_decode($response_body['response'], TRUE);
					if (!empty($upload_info['id']) && !empty($upload_info['snippet'])) {
						$keywords = Array(
								'youtube_video_id' => $upload_info['id'],
								'youtube_video_published_date' => $upload_info['snippet']['publishedAt'],
								'youtube_video_link'    => 'https://www.youtube.com/watch?v='.$upload_info['id'],
						);

						return TRUE;
					}
				}//end if
			}//end if
		}//end if

		if (!empty($response_body)) {
			$error_info = @json_decode($response_body, TRUE);
			if (isset($error_info['error'])) {
				trigger_error(sprintf(translate('YouTube file upload failed with error code %1$s: %2$s'), array_get_index($error_info['error'], 'code', 'n/a'), array_get_index($error_info['error'], 'message', '')));
			} else if (!empty($response_body) && is_string($response_body)) {
				trigger_error(sprintf(translate('YouTube file upload failed: %s'), $response_body));
			}
		}

		return FALSE;

	}//end processUploadVideo()


	/**
	* Fetch the loaded Oauth 2 access token from the Oauth Token asset
	*
	* @param $settings
	*
	* @return string
	* @access private
	*/
	private static function _getAccessToken($settings)
	{
		$access_token = '';
		$oauth2_token_assetid = array_get_index($settings, 'oauth2_token_assetid', '');

		if ($oauth2_token_assetid) {
			$oauth2_token = $GLOBALS['SQ_SYSTEM']->am->getAsset($oauth2_token_assetid, 'oauth2_token');
			$access_token_info = $oauth2_token->loadTokenFromStorage();
			$access_token = array_get_index($access_token_info, 'access_token', '');
		}

		return $access_token;

	}//end _getAccessToken()

}//end class

?>
