/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by ftapplicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <glib.h>
#include <dlog.h>
#include <app_common.h>
#include <sap.h>
#include <sap_file_transfer.h>
#include "ft_progressbar.h"
#include <string.h>
#include "service.h"

struct priv {
	sap_agent_h agent;
	sap_file_transaction_h socket;
};

static struct priv priv_data = { 0 };
static gboolean recognition_switch = 0; // 0이면 off, 1이면 on
gboolean file_on_progress = 0;

static void _on_send_completed(sap_file_transaction_h file_transaction,
			       sap_ft_transfer_e result,
			       const char *file_path,
			       void *user_data)
{
	char error_message[100];
	set_progress_bar_value(1);

	dlog_print(DLOG_INFO, LOG_TAG, "# transfer completed");

	if (priv_data.socket) {
		sap_file_transfer_destroy(file_transaction);
		priv_data.socket = NULL;
	}

	if (result == SAP_FT_TRANSFER_SUCCESS) {
		sprintf(error_message, "Transfer Completed");
	} else {
		switch (result) {
		case (SAP_FT_TRANSFER_FAIL_CHANNEL_IO): {
			sprintf(error_message, "Channel IO Error.");
			break;
		}

		case (SAP_FT_TRANSFER_FAIL_FILE_IO): {
			sprintf(error_message, "File IO Error.");
			break;
		}

		case (SAP_FT_TRANSFER_FAIL_CMD_DROPPED): {
			sprintf(error_message, "Transfer dropped/");
			break;
		}

		case (SAP_FT_TRANSFER_FAIL_PEER_UNRESPONSIVE): {
			sprintf(error_message, "Peer Un Responsive.");
			break;
		}

		case (SAP_FT_TRANSFER_FAIL_PEER_CONN_LOST): {
			sprintf(error_message, "Connection Lost.");
			break;
		}

		case (SAP_FT_TRANSFER_FAIL_PEER_CANCELLED): {
			sprintf(error_message, "Peer Cancelled.");
			break;
		}

		case (SAP_FT_TRANSFER_FAIL_SPACE_NOT_AVAILABLE): {
			sprintf(error_message, "No Space.");
			break;
		}

		default:
			sprintf(error_message, "Unknown Error");
		}

	}
	hide_progress_bar();
	file_on_progress = 0;
}

static void _on_sending_file_in_progress(sap_file_transaction_h file_transaction,
					 unsigned short int percentage_progress,
					 void *user_data)
{
	dlog_print(DLOG_INFO, LOG_TAG, "# progress %d", percentage_progress);
	set_progress_bar_value((float)percentage_progress / 100);
}

static void __set_file_transfer_cb(sap_file_transaction_h file_socket)
{
	dlog_print(DLOG_INFO, LOG_TAG, "# set callbacks");
	sap_file_transfer_set_progress_cb(file_socket, _on_sending_file_in_progress, NULL);
	sap_file_transfer_set_done_cb(file_socket, _on_send_completed, NULL);
	dlog_print(DLOG_INFO, LOG_TAG, "set file transfer cb pass");
}

void accept_file()
{
	int ret;
	char rcv_file_path[100];
	const char *data_path = NULL;
	const char *data2_path = NULL;
	dlog_print(DLOG_INFO, LOG_TAG, "come!!!!!!!!!");
	data_path = "/opt/usr/media/Downloads";
	data2_path = "/opt/usr/media/Downloads/woof/iconImage";
	const char *folder_path = NULL;
	folder_path = "/opt/usr/media/Downloads/woof";
	/*
	int nResult = mkdir( folder_path );

		if( nResult == 0 )
		{
			printf( "Woof 폴더 생성 성공" );
			int mResult = mkdir(data_path);
			if( mResult == 0 )
				{
					printf( "IconImage 폴더 생성 성공" );
				}
				else if( mResult == -1 )
				{
					perror( "폴더 생성 실패 - IconImage 폴더가 이미 있거나 부정확함\n" );
					printf( "errorno : %d", errno );
				}
		}
		else if( nResult == -1 )
		{
			perror( "폴더 생성 실패 - Woof 폴더가 이미 있거나 부정확함\n" );
			printf( "errorno : %d", errno );
		}
		*/
	DIR *dir = opendir(folder_path);
	//struct dirent *pDirent = NULL;
	if(chmod("/opt/usr/media", 0777) == -1) {
			dlog_print(DLOG_INFO, LOG_TAG, strerror(errno));
			dlog_print(DLOG_INFO, LOG_TAG, "media pass");
		}
	if(chmod(data_path, 0777) == -1) {
			dlog_print(DLOG_INFO, LOG_TAG, strerror(errno));
			dlog_print(DLOG_INFO, LOG_TAG, "downloads pass");
		}
	if(!dir) {
		if(mkdir(folder_path, 0755) == -1 && errno != EEXIST) {
			dlog_print(DLOG_INFO, LOG_TAG, strerror(errno));

		}

		dlog_print(DLOG_INFO, LOG_TAG, "woof success");

		dir = opendir(data2_path);
		if(!dir) {
			mkdir(data2_path, 0755);
			dlog_print(DLOG_INFO, LOG_TAG, "Image success");
		}
		else{
		dlog_print(DLOG_INFO, LOG_TAG, "Image fail");
		}
	}
		else{
	dlog_print(DLOG_INFO, LOG_TAG, "woof fail");
		}
	sprintf(rcv_file_path, "%s/downloadfile.jpg", data2_path);
	dlog_print(DLOG_INFO, LOG_TAG, rcv_file_path);
	//free(data_path);
	//free(data2_path);
	//free(folder_path);
	ret = sap_file_transfer_receive(priv_data.socket, rcv_file_path);

	dlog_print(DLOG_INFO, LOG_TAG, "rcv file success");
	file_on_progress = 1;

	show_progress_bar();
}

void reject_file()
{
	int ret;
	ret = sap_file_transfer_reject(priv_data.socket);

	file_on_progress = 0;
}

/*
 * 전송받으려하는 파일이름보고 인식인지 업데이트인지 체크하기위해 split
 */
void
switch_filepath(const char *file_path) {
	// 소리인식 알람요청 인 경우
	if(strstr(file_path, "recognition")) {
		recognition_switch = 1; // on
	}

	else {
		recognition_switch = 0; // off -> 업데이트
	}
}

static void _on_send_file(sap_peer_agent_h peer_agent_h,
			  sap_file_transaction_h socket,
			  const char *file_path,
			  void *user_data)
{
	file_on_progress = 1;
	priv_data.socket = socket;
	switch_filepath(file_path); // 소리 인식 여부 설정
	dlog_print(DLOG_INFO, LOG_TAG, "# incoming file request.");
	dlog_print(DLOG_INFO, LOG_TAG, file_path);
	__set_file_transfer_cb(priv_data.socket); // 파일 수신 준비

	// on인 경우(음성 인식)
	if(recognition_switch == 1)  {
		recognition_data_send(file_path);
	}
	// off인 경우(파일 전송)
	if(recognition_switch == 0) {
		__set_file_transfer_cb(priv_data.socket); // 파일 수신 준비
	}

}

void conn_terminated(sap_peer_agent_h peer_agent,
		     sap_socket_h socket,
		     sap_service_connection_terminated_reason_e result,
		     void *user_data)
{
	dlog_print(DLOG_INFO, LOG_TAG, "connection terminated");
}

static void on_conn_req(sap_peer_agent_h peer_agent,
			sap_socket_h socket,
			sap_service_connection_result_e result,
			void *user_data)
{
	sap_peer_agent_accept_service_connection(peer_agent);
	sap_peer_agent_set_service_connection_terminated_cb(peer_agent, conn_terminated, NULL);
}

static void on_agent_initialized(sap_agent_h agent,
				 sap_agent_initialized_result_e result,
				 void *user_data)
{
	switch (result) {
	case SAP_AGENT_INITIALIZED_RESULT_SUCCESS:

		dlog_print(DLOG_DEBUG, LOG_TAG, "agent is initialized");

		priv_data.agent = agent;

		sap_file_transfer_set_incoming_file_cb(agent, _on_send_file, NULL);
		dlog_print(DLOG_INFO, LOG_TAG, "sap file transfer set incoming file cb pass");
		sap_agent_set_service_connection_requested_cb(agent, on_conn_req, NULL);
		dlog_print(DLOG_INFO, LOG_TAG, "sap_agent_set_service_connection_requested_cb pass");
		dlog_print(DLOG_INFO, LOG_TAG, "sap agent initialized result success pass");
		break;

	case SAP_AGENT_INITIALIZED_RESULT_DUPLICATED:
		dlog_print(DLOG_ERROR, LOG_TAG, "duplicate registration");

		break;

	case SAP_AGENT_INITIALIZED_RESULT_INVALID_ARGUMENTS:
		dlog_print(DLOG_ERROR, LOG_TAG, "invalid arguments");

		break;

	case SAP_AGENT_INITIALIZED_RESULT_INTERNAL_ERROR:
		dlog_print(DLOG_ERROR, LOG_TAG, "internal sap error");

		break;

	default:
		dlog_print(DLOG_ERROR, LOG_TAG, "unknown status (%d)", result);

		break;
	}
}

static void _on_device_status_changed(sap_device_status_e status,
				      sap_transport_type_e transport_type,
				      void *user_data)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "%s, status :%d", __func__, status);

	switch (transport_type) {
	case SAP_TRANSPORT_TYPE_BT:
		dlog_print(DLOG_DEBUG, LOG_TAG, "transport_type (%d): bt", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_BLE:
		dlog_print(DLOG_DEBUG, LOG_TAG, "transport_type (%d): ble", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_TCP:
		dlog_print(DLOG_DEBUG, LOG_TAG, "transport_type (%d): tcp/ip", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_USB:
		dlog_print(DLOG_DEBUG, LOG_TAG, "transport_type (%d): usb", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_MOBILE:
		dlog_print(DLOG_DEBUG, LOG_TAG, "transport_type (%d): mobile", transport_type);
		break;

	default:
		dlog_print(DLOG_ERROR, LOG_TAG, "unknown transport_type (%d)", transport_type);
		break;
	}

	switch (status) {
	case SAP_DEVICE_STATUS_DETACHED:
		dlog_print(DLOG_DEBUG, LOG_TAG, "device is not connected.");



		break;

	case SAP_DEVICE_STATUS_ATTACHED:
		dlog_print(DLOG_DEBUG, LOG_TAG, "Attached calling find peer now");



		break;

	default:
		dlog_print(DLOG_ERROR, LOG_TAG, "unknown status (%d)", status);
		break;
	}
}

void cancel_file()
{
	sap_file_transfer_cancel(priv_data.socket);
	hide_progress_bar();
}

gboolean agent_initialize()
{
	int result = 0;
	dlog_print(DLOG_DEBUG, LOG_TAG, "11111111111111.");
	do {
		result = sap_agent_initialize(priv_data.agent, "/sample/filetransfer", SAP_AGENT_ROLE_PROVIDER,
					      on_agent_initialized, NULL);

		dlog_print(DLOG_DEBUG, LOG_TAG, "SAP >>> getRegisteredServiceAgent() >>> %d", result);
	} while (result != SAP_RESULT_SUCCESS);
	dlog_print(DLOG_DEBUG, LOG_TAG, "22222222222222.");
	return TRUE;
}

gboolean initialize_sap(void)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "initialize sap go");
	sap_agent_h agent = NULL;

	sap_agent_create(&agent);

	priv_data.agent = agent;
	dlog_print(DLOG_DEBUG, LOG_TAG, "initialize sap go2");
	agent_initialize();

	sap_set_device_status_changed_cb(_on_device_status_changed, NULL);

	return TRUE;
}
