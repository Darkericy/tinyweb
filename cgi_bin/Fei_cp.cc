#include "../DK.h"

int main(){
	ostringstream output;

	//输入响应报头
	output << "Connection: close\r\n";
	output << "Content-type: text/html\r\n\r\n";
	cout << output.str();
	
	vector<int> dp(15);
	dp[0] = 1, dp[1] = 1;
	for(int i = 0; i < 15; ++i){
		if(i > 1){
			dp[i] = dp[i - 1] + dp[i - 2];
		}
		cout << dp[i] << " " << endl;
		this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return 0;
}

