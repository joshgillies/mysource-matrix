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
* $Id: trigger_action_update_twitter_status.inc,v 1.9 2012/12/12 03:14:52 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_ATTRIBUTES_PATH.'/oauth/oauth.inc';

// Normaliser PEAR class
require_once 'I18N/UnicodeNormalizer.php';


/**
* Trigger_Action_Update_Twitter_Status
*
* Purpose
*    Update the status of a Twitter account.
*
*
* @author  Luke Wright <lwright@squiz.com.au>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Update_Twitter_Status extends Trigger_Action
{


	/**
	* Execute this action
	*
	* Returns an array of data about what it did, or false on error
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		if ($state['asset']) {
			$asset = $state['asset'];
		} else if ($state['assetid']) {
			$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($state['assetid']);
		}

		if (empty($settings['access_token']) === TRUE) {
			// No access token
			trigger_localised_error('CORE0289', E_USER_WARNING);
			return FALSE;
		}

		// Get our keyword replacements
		$tweet_format = $settings['tweet_contents'];
		$asset_keywords = retrieve_keywords_replacements($tweet_format);
		$replacements = Array();
		foreach ($asset_keywords as $keyword) {
			$additional_rep = self::getAdditionalKeywordReplacement($asset, $keyword);
			if ($additional_rep !== NULL) {
				$replacements[$keyword] = $additional_rep;
			} else {
				$replacements[$keyword] = $state['asset']->getKeywordReplacement($keyword);
			}
		}//end foreach
		replace_keywords($tweet_format, $replacements);
		replace_global_keywords($tweet_format);

		if (strlen($tweet_format) === 0) {
			trigger_localised_error('CORE0290', E_USER_WARNING);
			return FALSE;
		}

		// Tweets need to be UTF-8 encoded, no matter what
		if (mb_detect_encoding($tweet_format) !== 'UTF-8') {
			$tweet_format = mb_convert_encoding($tweet_format, 'UTF-8');
		}

		// Move
		$tweet_format = I18N_UnicodeNormalizer::toNFC($tweet_format);
		$tweet_length = mb_strlen($tweet_format, 'UTF-8');

		// If the normalised size of the tweet is more than 140, it's too long
		switch ($settings['if_tweet_too_long']) {
			case 'fail':
				if ($tweet_length > 140) {
					trigger_localised_error('CORE0291', E_USER_WARNING);
					return FALSE;
				}

			break;

			case 'abbrev':
				$tweet_format = self::abbreviateStatus($tweet_format);
				$tweet_length = mb_strlen($tweet_format, 'UTF-8');

				// It's still too long?
				if ($tweet_length > 140) {
					trigger_localised_error('CORE0292', E_USER_WARNING);
					return FALSE;
				}

			break;

		}//end switch

		$settings['tweet_contents'] = $tweet_format;
		$return = self::processUpdateStatus($settings);

		$result = json_decode_array($return);

		if (isset($result['errors'])) {
			trigger_localised_error('CORE0293', E_USER_WARNING, (string)$result['errors'][0]['message']);
			return FALSE;
		} else {
			// Get the time that Twitter reported it as
			$tweet_time = strtotime((string) $result['created_at']);
		}

		return Array(
				'status'	=> $tweet_format,
				'time'		=> $tweet_time,
			   );

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
	public static function getInterface($settings, $prefix, $write_access=FALSE, Trigger $trigger=NULL, $action_id=NULL)
	{

		$settings['tweet_contents'] = array_get_index($settings, 'tweet_contents', '');
		$settings['if_tweet_too_long'] = array_get_index($settings, 'if_tweet_too_long', 'abbrev');

		$settings['consumer_key']    = array_get_index($settings, 'consumer_key');
		$settings['consumer_secret'] = array_get_index($settings, 'consumer_secret');
		$settings['request_token']   = array_get_index($settings, 'request_token');
		$settings['request_token_secret'] = array_get_index($settings, 'request_token_secret');
		$settings['access_token']    = array_get_index($settings, 'access_token');
		$settings['access_token_secret'] = array_get_index($settings, 'access_token_secret');
		$settings['access_token_acct_for'] = array_get_index($settings, 'access_token_acct_for');

		if ((empty($_REQUEST['oauth_token']) === FALSE) && (empty($_REQUEST['oauth_verifier']) === FALSE)) {
			// Check that this is the request for this token (and therefore,
			// for this trigger action where there is more than one)
			if ($settings['request_token'] === $_REQUEST['oauth_token']) {
				if ($write_access === TRUE) {
					$trigger_actions = $trigger->attr('actions');
					self::processCallback($settings, $_REQUEST['oauth_verifier']);
					$trigger_actions[$action_id]['data'] = $settings;
					$trigger->setAttrValue('actions', $trigger_actions);
					$trigger->saveAttributes();
				}
			}
		}

		if ((empty($_REQUEST['denied']) === FALSE)) {
			// Our request token was denied (!) for some reason. Twitter
			// returns a "denied" param in the link it gives to callback (it
			// does not callback automatically).
			if ($settings['request_token'] === $_REQUEST['denied']) {
				trigger_localised_error('CORE0294', E_USER_WARNING);
			}
		}

		// If we have an access token, ensure that we can still actually post
		// with it!
		// But calls to verifying credentials comes off our rate limit, so cache
		// it momentarily (for 1 minute) so it doesn't use too many hits.
		if (empty($settings['access_token']) === FALSE) {
			$cache_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cache_manager');
			$cache_key = strtolower(__CLASS__).'_creds_verified_'.$settings['access_token'];

			$result = $cache_mgr->loadFromCache($trigger->id, 'trigger', $cache_key, FALSE);

			if ($result !== '1') {
				$creds_response = self::processVerify($settings);
				$response = json_decode_array($creds_response);


				if (isset($response['error'])) {
					// No longer authenticated!
					$cache_mgr->saveToCache($trigger->id, 'trigger', $cache_key, '0', FALSE, 1);

					// Authentication issue? We've probably been revoked.
					if (((string)$response['error']) === 'Could not authenticate you.') {
						$settings['access_token'] = NULL;
						$settings['access_token_secret'] = NULL;
						trigger_localised_error('CORE0295', E_USER_NOTICE);
					} else {
						// Think it's a temporary issue
						trigger_localised_error('CORE0296', E_USER_NOTICE, (string)$response['error']);
					}
				} else {
					$cache_mgr->saveToCache($trigger->id, 'trigger', $cache_key, '1', FALSE, 60);
				}

			}
		}

		$credentials_provided = ((empty($settings['consumer_key']) === FALSE) && (empty($settings['consumer_secret']) === FALSE)) ? TRUE : FALSE;
		$consumer_details_locked = ((empty($settings['access_token']) === TRUE) && (empty($_REQUEST['oauth_verifier']) === TRUE)) ? FALSE : TRUE;

		// If the client credentials are there and we aren't in the middle of
		// authentication (or completed), refresh the request token
		if (($credentials_provided === TRUE) && ($consumer_details_locked === FALSE)) {
			if ($write_access === TRUE) {
				$trigger_actions = $trigger->attr('actions');
				self::processRequestToken($settings);
				$trigger_actions[$action_id]['data'] = $settings;
				$trigger->setAttrValue('actions', $trigger_actions);
				$trigger->saveAttributes();
			}
		}


		$authorised = FALSE;

		ob_start();

		hidden_field($prefix.'[request_token]', $settings['request_token']);
		hidden_field($prefix.'[request_token_secret]', $settings['request_token_secret']);
		hidden_field($prefix.'[access_token]', $settings['access_token']);
		hidden_field($prefix.'[access_token_secret]', $settings['access_token_secret']);
		hidden_field($prefix.'[access_token_acct_for]', $settings['access_token_acct_for']);
		?>
		<div>
			<p class="sq-backend-section-subheading"><?php echo translate('trigger_action_update_twitter_status_register_system_header') ?></p>

			<?php echo translate('trigger_action_update_twitter_status_register_system_note') ?>
		</div>

		<div>
			<p class="sq-backend-section-subheading"><?php echo translate('trigger_action_update_twitter_status_client_credentials_header') ?></p>

			<table class="sq-backend-table with-bottom-margin">
			<thead>
				<colgroup>
					<col width="20%" />
					<col width="80%" />
				</colgroup>
			</thead>
			<tbody>
				<tr>
					<th><?php echo translate('trigger_action_update_twitter_status_client_credentials_key') ?></th>
					<td><?php
					if (($write_access === TRUE) && ($consumer_details_locked === FALSE)) {
						text_box($prefix.'[consumer_key]', $settings['consumer_key'], 50, 0);
					} else {
						echo $settings['consumer_key'];
						hidden_field($prefix.'[consumer_key]', $settings['consumer_key']);
					}
					?></td>
				</tr>
				<tr>
					<th><?php echo translate('trigger_action_update_twitter_status_client_credentials_secret') ?></th>
					<td><?php
					if (($write_access === TRUE) && ($consumer_details_locked === FALSE)) {
						text_box($prefix.'[consumer_secret]', $settings['consumer_secret'], 50, 0);
					} else {
						echo $settings['consumer_secret'];
						hidden_field($prefix.'[consumer_secret]', $settings['consumer_secret']);
					}
					?></td>
				</tr>
				<tr>
					<th><?php echo translate('trigger_action_update_twitter_status_client_credentials_status') ?></th>
					<td><?php
					if ((empty($settings['consumer_key']) === TRUE) || (empty($settings['consumer_secret']) === TRUE)) {
						echo translate('trigger_action_update_twitter_status_client_credentials_status_no_consumer');
					} else {
						// We have a consumer key
						if (empty($settings['access_token']) === FALSE) {
							echo sprintf(translate('trigger_action_update_twitter_status_client_credentials_status_yes'), $settings['access_token_acct_for']);
							echo '<br/>';
							check_box($prefix.'[deauthorise]');
							?> <label for="<?php echo $prefix ?>[deauthorise]"><?php
							echo translate('trigger_action_update_twitter_status_client_credentials_deauthorise');
						} else if (empty($settings['request_token']) === FALSE) {
							if ($write_access === TRUE) {
								$url = 'https://api.twitter.com/oauth/authenticate?oauth_token='.$settings['request_token'].'&force_login=true';
								echo sprintf(translate('trigger_action_update_twitter_status_client_credentials_status_no'), $url);
							} else {
								echo translate('trigger_action_update_twitter_status_client_credentials_status_no_unlocked');
							}
						} else {
							?><strong style="color: #f00">Error: failed to gain request token</strong><?php
							foreach ($settings['request_token_errors'] as $error) {
								?><br/><?php echo $error ?><?php
							}
							?></em><?php
						}
					}
					?></td>
				</tr>
				</tbody>
			</table>
		</div>

		<div>
			<?php echo translate('trigger_action_update_twitter_status_client_credentials_note') ?>
		</div>

		<div>
			<p class="sq-backend-section-subheading"><?php echo translate('trigger_action_update_twitter_status_tweet_format_header') ?></p>

			<table class="sq-backend-table with-bottom-margin">
				<thead>
				<colgroup>
					<col width="20%" />
					<col width="80%" />
				</colgroup>
				</thead>
				<tbody>
				<tr>
					<th><?php echo translate('trigger_action_update_twitter_status_tweet_format_format') ?></th>
					<td><?php
					echo '<span>';
					if ($write_access === TRUE) {
						text_box($prefix.'[tweet_contents]', $settings['tweet_contents'], 70, 0, FALSE, 'onkeyup="document.getElementById(\''.$prefix.'_tweet_length\').innerHTML = (140 - this.value.length).toString(); return true;"');
					} else {
						echo $settings['tweet_contents'];
						hidden_field($prefix.'[tweet_contents]', $settings['tweet_contents']);
					}?> <?php echo sprintf(translate('trigger_action_update_twitter_status_tweet_format_char_count'), $prefix, 140 - strlen($settings['tweet_contents']));
					echo '</span>';
					?>
					</td>
				</tr>
				<tr>
					<th><?php echo translate('trigger_action_update_twitter_status_tweet_format_iftoolong') ?></th>
					<td><?php
					$options = Array(
								'abbrev'	=> translate('trigger_action_update_twitter_status_tweet_format_iftoolong_option_abbrev'),
								'fail'		=> translate('trigger_action_update_twitter_status_tweet_format_iftoolong_option_fail'),
							   );
					if ($write_access === TRUE) {
						combo_box($prefix.'[if_tweet_too_long]', $options, FALSE, $settings['if_tweet_too_long']);
					} else {
						echo $options[$settings['if_tweet_too_long']];
						hidden_field($prefix.'[if_tweet_too_long]', $settings['if_tweet_too_long']);
					}?></td>
				</tr>
				</tbody>
			</table>
		</div>

		<div>
			<?php echo translate('trigger_action_update_twitter_status_tweet_format_note') ?>
		</div>

		<?php

		$contents = ob_get_clean();

		return $contents;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$settings['tweet_contents'] = array_get_index($request_data, 'tweet_contents');
		$settings['if_tweet_too_long'] = array_get_index($request_data, 'if_tweet_too_long');
		$settings['consumer_key'] = array_get_index($request_data, 'consumer_key');
		$settings['consumer_secret'] = array_get_index($request_data, 'consumer_secret');
		$settings['request_token'] = array_get_index($request_data, 'request_token');
		$settings['request_token_secret'] = array_get_index($request_data, 'request_token_secret');
		$settings['access_token'] = array_get_index($request_data, 'access_token');
		$settings['access_token_secret'] = array_get_index($request_data, 'access_token_secret');
		$settings['access_token_acct_for'] = array_get_index($request_data, 'access_token_acct_for');

		$oauth_verifier = array_get_index($request_data, 'oauth_verifier');
		$deauthorise = (boolean)array_get_index($request_data, 'deauthorise', FALSE);

		if ($deauthorise === TRUE) {
			// Blank out current tokens
			$settings['request_token'] = NULL;
			$settings['request_token_secret'] = NULL;
			$settings['request_token_errors'] = NULL;

			$settings['access_token'] = NULL;
			$settings['access_token_secret'] = NULL;
			$settings['access_token_errors'] = NULL;
		}

		return FALSE;

	}//end processInterface()


	/**
	* Function that handles callback
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processRequestToken(&$settings)
	{
		// Set the new consumer settings
		$callback_url = replace_query_string_vars(Array(), NULL, NULL, TRUE);
		$additional_params = 'SQ_BACKEND_PAGE=frames';
		if (strpos($callback_url, '?') === FALSE) {
			$callback_url .= '?'.$additional_params;
		} else {
			$callback_url .= '&'.$additional_params;
		}

		$oauth_attr = new Asset_Attribute_OAuth();
		$oauth_attr_values = Array(
								'consumer_key'		=> $settings['consumer_key'],
								'consumer_secret'	=> $settings['consumer_secret'],
								'request_token_url'	=> 'https://api.twitter.com/oauth/request_token',
								'callback_url'		=> $callback_url,
								'method'			=> 'GET',
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

		if ($result === NULL) {
			$settings['request_token_errors'] = $oauth_attr->getErrors();
		} else {
			$settings['request_token'] = $result['request_token']['oauth_token'];
			$settings['request_token_secret'] = $result['request_token']['oauth_token_secret'];
		}

		return;

	}//end processRequestToken()


	/**
	* Function that handles callback
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processCallback(&$settings, $oauth_verifier)
	{
		// Did we get back the token we were expecting?
		$oauth_attr = new Asset_Attribute_OAuth();
		$oauth_attr_values = Array(
								'consumer_key'		=> $settings['consumer_key'],
								'consumer_secret'	=> $settings['consumer_secret'],
								'access_token_url'	=> 'https://api.twitter.com/oauth/access_token',
								'method'			=> 'GET',
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
		$result = $oauth_attr->getAccessToken($settings['request_token'], $oauth_verifier, $settings['request_token_secret']);
		if ($result === NULL) {
			$settings['access_token_errors'] = $oauth_attr->getErrors();
		} else {
			$settings['access_token'] = $result['access_token']['oauth_token'];
			$settings['access_token_secret'] = $result['access_token']['oauth_token_secret'];
			$settings['access_token_acct_for'] = $result['access_token']['screen_name'];
		}

		return FALSE;

	}//end processCallback()


	/**
	* Calls verifying of credentials
	*
	* @param array	&$settings		a container for any data the action might want to save
	*
	* @return boolean
	* @access public
	*/
	public static function processVerify($settings)
	{
		// Did we get back the token we were expecting?
		$oauth_attr = new Asset_Attribute_OAuth();
		$oauth_attr_values = Array(
								'consumer_key'		=> $settings['consumer_key'],
								'consumer_secret'	=> $settings['consumer_secret'],
								'method'			=> 'GET',
								'signature_method'	=> 'HMAC-SHA1',
								'request_headers'	=> Array(),
								'request_body'		=> '',
								'timeout'			=> 30,
								'cache_options'		=> 'NEVER',
								'follow_redirect'	=> FALSE,
							 );
		$oauth_attr->setValue($oauth_attr_values);

		// Grab a new request token
		$url = 'https://api.twitter.com/1.1/account/verify_credentials.json';
		$header = $oauth_attr->getUserDataAuthHeader($url, $settings['access_token'], $settings['access_token_secret']);

		$details = fetch_url($url, array('RETURNTRANSFER' => 1), array($header));

		return $details['response'];

	}//end processVerify()


	/**
	* Function that handles callback
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processUpdateStatus($settings)
	{
		// Did we get back the token we were expecting?
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

		$oauth_attr_values = $oauth_attr->value;

		// Get the URL for status updating - but we need to hack around this,
		// since OAuth attr only supports adding new fields through query params
		// so get it to sign using that
		$settings['tweet_contents'] = str_replace('+', ' ', str_replace('%7E', '~', rawurlencode($settings['tweet_contents'])));
		$url = 'https://api.twitter.com/1.1/statuses/update.json';
		$url_for_signing = $url.'?status='.$settings['tweet_contents'];

		// It also doesn't seem to like POST. So we will get the header as
		// usual, then break it apart and turn it into POST data.
		$header = $oauth_attr->getUserDataAuthHeader($url_for_signing, $settings['access_token'], $settings['access_token_secret'], FALSE);
		$header = substr_replace($header, '', 0, strlen('Authorization: OAuth '));

		$post_items = Array();
		$post_bits = Array();

		$found = preg_match_all('/([^,=]*)="([^"]*)"/U', $header, $matches, PREG_SET_ORDER);
		foreach ($matches as $match) {
			$key   = $match[1];
			$value = $match[2];
			$post_items[$key] = $value;
		}

		$post_items['status'] = $settings['tweet_contents'];

		foreach ($post_items as $key => $value) {
			$post_bits[] = $key.'='.$value;
		}

		$post_data = implode('&', $post_bits);

		$curl_options = array(
				'RETURNTRANSFER' => 1,
				'POST'           => 1,
				'POSTFIELDS'     => $post_data,
		);

		$details = fetch_url($url, $curl_options);

		return $details['response'];

	}//end processUpdateStatus()


	/**
	* Get an additional keyword replacement
	*
	* We provide one additional keyword:
	*
	* @param Asset	$asset		The asset to derive a keyword replacement from
	* @param string	$keyword	The keyword to get a replacement for
	*
	* @return string
	*/
	public static function getAdditionalKeywordReplacement(Asset $asset, $keyword)
	{
		$replacement = NULL;

		switch ($keyword)
		{
			// This keyword will get the shortest URL to the
			case 'asset_short_url':
				$urls = Array();

				// Get the current (closest) URL of the firing asset
				$current_url = $asset->getUrl();

				if (empty($current_url) === FALSE) {
					$urls[] = $current_url;

					// Then find URLs that remap to this URL
					$remap_mgr = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('remap_manager');
					$remapped_urls = $remap_mgr->getRemapURLs($asset);

					foreach ($remapped_urls as $url_item) {
						if ($url_item['remap_url'] === $current_url) {
							$urls[] = $url_item['url'];
						}
					}

					usort($urls, create_function('$a,$b', 'return strlen($a) - strlen($b);'));

					$replacement = reset($urls);
				}

			break;

		}//end switch

		return $replacement;

	}//end getAdditionalKeywordReplacement()


	/**
	* Abbreviate status to 140 characters.
	*
	* Status string will be broken up into "url links" and "not links". Any
	* whitespace between these will be reduced to one space only.
	*
	* Links will NOT be shortened, for obvious reasons. You are advised to use
	* the "Add Remap" trigger action to create a shorter remap, and use the
	* %asset_short_url% keyword.
	*
	* Text around links will be ellipsised where necessary to save space, from
	* the largest amount of text first. This allows you to have a "name,
	* description and link" that will ellipsise as expected.
	* However, it will be shortened to a minimum of 10 characters only
	* (eg. "1234567..."). If we cannot shorten it, we will return what we can.
	*
	* @param string	$status	The full status string
	*
	* @return string
	*/
	public static function abbreviateStatus($status)
	{
		// Link detection. The pattern is taken from valid_url() in Fudge's
		// general/www.inc, but without the start/end-string restriction
		$pattern = '/\s+(([A-Za-z0-9\-\.\+]+):\/\/[a-z0-9]+(([\-\.]{1}[a-z0-9]+)*\.[a-z]{2,})?((:[0-9]{1,5})?\/[^\s]*)?)\s+/i';
		$search_status = ' '.$status.' ';

		$string_bits = Array();

		$found = preg_match($pattern, $search_status, $matches, PREG_OFFSET_CAPTURE);
		$last_offset = 1;

		while ($found > 0) {
			$start_pos = $matches[1][1];
			$length = mb_strlen($matches[1][0]);

			if ($start_pos > 0) {
				// normal text before the link
				$string_bits[] = Array(
									'text'	=> trim(mb_substr($search_status, 0, $start_pos)),
								 );
			}

			// the link itself
			$string_bits[] = Array(
								'link'	=> mb_substr($search_status, $start_pos, $length),
							 );

			// Get the next link (if any)
			$search_status = mb_substr($search_status, $start_pos + $length);
			$found = preg_match($pattern, $search_status, $matches, PREG_OFFSET_CAPTURE);

		}///end while

		// Is there anything else after?
		$search_status = trim($search_status);
		if (mb_strlen($search_status) > 0) {
			$string_bits[] = Array(
								'text'	=> $search_status,
							 );
		}

		// Calculate char count, and work out the size of each text bit so
		// we know which one to shorten first
		$char_count = count($string_bits) - 1;
		$text_bits_lengths = Array();

		foreach ($string_bits as $string_key => $string_bit) {
			reset($string_bit);

			switch (key($string_bit)) {
				case 'text':
					$text_bits_lengths[$string_key] = mb_strlen($string_bit['text']);
					// deliberate fall-through...

				case 'link':
					$char_count += mb_strlen($string_bit[key($string_bit)]);
				break;
			}
		}

		// Are there any text bits at all?
		if (count($text_bits_lengths) > 0) {

			arsort($text_bits_lengths);
			reset($text_bits_lengths);
			$excess = $char_count - 140;
			$modified = FALSE;

			while ($excess > 0) {
				// Sort the text bits by their length, descending, so we have the
				// largest text bits first
				$key = key($text_bits_lengths);
				$size = current($text_bits_lengths);

				// Try to make it the largest of: 10 characters, half current size,
				// or whatever we are over.
				$new_size = max(10, max($size / 2, $size - $excess));

				if ($new_size < $size) {
					$string_bits[$key]['text'] = mb_substr($string_bits[$key]['text'], 0, $new_size - 3).'...';
					$modified = TRUE;

					$excess -= ($size - $new_size);
					$text_bits_lengths[$key] = $new_size;
				}

				next($text_bits_lengths);

				// Go around again if we've hit the smallest bit, unless the
				// string didn't get any shorter - in that case, give up :(
				if (key($text_bits_lengths) === NULL) {
					if ($modified === FALSE) break;
					$modified = FALSE;

					// Re-sort the lengths
					arsort($text_bits_lengths);
					reset($text_bits_lengths);
				}

			}//end while

		}

		// Put it all back together again, whether we've abbreviated or not
		foreach ($string_bits as &$string_bit) {
			$string_bit = $string_bit[key($string_bit)];
		}
		unset($string_bit);

		return implode(' ', $string_bits);

	}//end abbreviateStatus()


}//end class

?>
