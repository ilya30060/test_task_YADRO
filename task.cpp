#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <queue>
#include <vector>
#include <list>
#include <regex>
#include <memory>
#include <algorithm>
class Computer_Club;
bool compare(const std::string& left, const std::string& right)
{
	return left.length() < right.length();
}
int get_Minutes(std::string& timestamp); 
int get_Hours(std::string& timestamp);

class Computer_Club {

	class Computer;
	class Client;
	class Computer {
	public:
		Computer(Computer_Club& parent_club, int id) :
			_cash_amount{ 0 }, _id{ id },
			_total_busy_time_in_minutes{ 0 },
			_current_client{ nullptr },
			_timestamp_prev_login{},
			_available{ true },
			_parent_club{ parent_club } {}

		bool isFree() {
			return this->_available;
		}
		int getId() {
			return this->_id;
		}
		void tookBy(std::shared_ptr<Client> client, std::string time) {
			_current_client = client;
			_current_client->sitAt(_parent_club.getComputerById(this->_id));
			_available = false;
			_timestamp_prev_login = time;
			
		}
		void makeFree(std::string& time) {
			if (_current_client != nullptr) {
				int minutes_start = get_Hours(_timestamp_prev_login) * 60 + get_Minutes(_timestamp_prev_login);
				int minutes_finish = get_Hours(time) * 60 + get_Minutes(time);
				int minutes_amount = minutes_finish - minutes_start;
				_total_busy_time_in_minutes += minutes_amount;
				int hours_amount = minutes_amount / 60;
				minutes_amount -= 60 * hours_amount;
				if (minutes_amount > 0) hours_amount++;
				_cash_amount += hours_amount * _parent_club._hour_price;
				std::shared_ptr<Client> leaving_client = _current_client;
				_current_client = nullptr;
				_available = true;
				leaving_client->standUp(time);
				if (_parent_club.queueSize() > 0) {
					this->tookBy(_parent_club.getFromQueue(), time);
					_parent_club.output.push_back(time + ' ' + std::to_string(12) + ' ' + _current_client->getName() + ' ' + std::to_string(this->getId()));
				}
			}
		}
		int getCashAmount() {
			return _cash_amount;
		}
		std::string getAllWorkTime() {
			int hours = this->_total_busy_time_in_minutes / 60;
			_total_busy_time_in_minutes -= 60 * hours;
			int minutes = _total_busy_time_in_minutes;
			std::string hours_string{};
			std::string minutes_string{};
			if (hours < 10) hours_string = "0" + std::to_string(hours);
			else hours_string = std::to_string(hours);
			if (minutes < 10) minutes_string = "0" + std::to_string(minutes);
			else minutes_string = std::to_string(minutes);
			std::string temp{ hours_string + ':' + minutes_string };
			return temp;
		}
	private:
		int _cash_amount;
		int _id;
		int _total_busy_time_in_minutes;
		std::shared_ptr<Client> _current_client;
		std::string _timestamp_prev_login;
		//0 - busy
		//1 - free
		bool _available;
		Computer_Club& _parent_club;
	};
	class Client {
		std::string _name;
		std::shared_ptr<Computer> _current_computer;
		
	public:
		Client(std::string& name): _name{name}, _current_computer{nullptr}
		{}
		std::string& getName() {
			return _name;
		}
		void sitAt(std::shared_ptr<Computer> computer) {
			if (computer->isFree()) _current_computer = computer;
			else throw " PlaceIsBusy"; //ğåøèë èçëèøíå ïåğåñòğàõîâàòüñÿ
		}
		bool isSitted() {
			return _current_computer != nullptr ? true : false;
		}
		void standUp(std::string& time) {
			if (_current_computer != nullptr) {
				std::shared_ptr<Computer> leaving_computer = _current_computer;
				_current_computer = nullptr;
				leaving_computer->makeFree(time);
			}
		}
		std::shared_ptr<Computer> getCurrentComputer() {
			return _current_computer;
		}
	};
public:
	class Event {
		Computer_Club& _parent_club;
		int _id;
		std::string _time;
		std::string _client_name;
		int _target_comp;
	public:
		Event(Computer_Club& parent_club, std::string& time, int id, std::string& client_name, int target_comp = 0) :   _parent_club{ parent_club },
																														_time { time },
																														_id{ id }, 
																														_client_name{ client_name }, 
																														_target_comp{target_comp}
		{
		}
		void resolve() {
			if (_id == 2) {
				_parent_club.output.push_back(_time + ' ' + std::to_string(_id) + ' ' + _client_name + ' ' + std::to_string(_target_comp));
			}
			else {
				_parent_club.output.push_back(_time + ' ' + std::to_string(_id) + ' ' + _client_name);
			}
			try {
				switch (_id) {
					case 1:
						if (_time < _parent_club.getTimeBegin()) {
							throw " NotOpenYet";
						}
						if (_parent_club.isClientExist(_client_name)) {
							throw " YouShallNotPass";
						}
						_parent_club.clientCome(_client_name);
						break;
					case 2:
						if (!_parent_club.isClientExist(_client_name)) {
							throw " ClientUnknown";
						}
						if (!_parent_club.isComputerExist(_target_comp)) {
							std::cout << _time + ' ' + std::to_string(_id) + ' ' + _client_name + ' ' + std::to_string(_target_comp);
							throw 1;
						}
						if (_parent_club.getComputerById(_target_comp)->isFree()) {
							if (_parent_club.getClientByName(_client_name)->isSitted()) {//ìåõàíèçì ïåğåñàäêè
								_parent_club.getClientByName(_client_name)->getCurrentComputer()->makeFree(_time);
							}
							_parent_club.getComputerById(_target_comp)->tookBy(_parent_club.getClientByName(_client_name), _time);
						}
						else {
							throw " PlaceIsBusy";
						}
						break;
					case 3:
						if (_parent_club.isAnyFreeComputer()) throw " ICanWaitNoLonger!";
						if (_parent_club.queueSize() < _parent_club.getComputersAmount()) {
							//äîáàâëåíèå â î÷åğåäü
							_parent_club.addInQueue(_client_name);
						}
						else {
							_parent_club.output.push_back(_time + ' ' + std::to_string(11) + ' ' + _client_name);
							_parent_club.clientLeave(_client_name, _time);
						}
						break;
					case 4:
						if (!_parent_club.isClientExist(_client_name)) throw " ClientUnknown";
						_parent_club.clientLeave(_client_name, _time);
						break;
				}
			}
			catch (const char* error_code) {
				_parent_club.output.push_back(_time + ' ' + std::to_string(13) + error_code);//ÇÄÅÑÜ ÍÅÒ ÏĞÎÁÅËÀ ÏÅĞÅÄ ÊÎÄÎÌ ÎØÈÁÊÈ. ÏÎÄĞÀÇÓÌÅÂÀÅÒÑß, ×ÒÎ ÊÎÄ ÎØÈÁÊÈ ÓÆÅ ÑÎÄÅĞÆÈÒ ÏĞÎÁÅË ÂÍÀ×ÀËÅ (ÏĞÈ ÍÅÎÁÕÎÄÈÌÎÑÒÈ ÌÎÆÍÎ ÈÇÌÅÍÈÒÜ)
			}
		}
	};

	std::shared_ptr<Client> getClientByName(std::string name) {
		std::shared_ptr<Client> return_client;
		for (std::shared_ptr<Client> client : _clients) {
			if (client->getName() == name)
			{
				return_client = client;
				break;
			}
		}
		return return_client;
	}
	std::shared_ptr<Computer> getComputerById(int id) {
		std::shared_ptr<Computer> return_computer;
		for (std::shared_ptr<Computer> computer : _computers) {
			if (computer->getId() == id)
			{
				return_computer = computer;
				break;
			}
		}
		return return_computer;
	}
	void clientCome(std::string& name) {
		_clients.push_back(std::make_shared<Client>(name));
	}
	void clientLeave(std::string& name, std::string& time) {
		for (std::list<std::shared_ptr<Client>>::iterator client = _clients.begin(); client != _clients.end(); ++client) {
			if ((*client)->getName() == name) {
				(*client)->standUp(time);
				_clients.erase(client);
				break;
			}
		}
	}
	bool isClientExist(std::string& name) {
		for (std::shared_ptr<Client> client : _clients) {
			if (client->getName() == name) return true;
		}
		return false;
	}
	bool isComputerExist(int id) {
		for (std::shared_ptr<Computer> computer : _computers) {
			if (computer->getId() == id) return true;
		}
		return false;
	}
	bool isAnyFreeComputer() {

		for (std::shared_ptr<Computer> computer : _computers) {
			if (computer->isFree()) return true;
		}
		return false;
	}
	void run() {
		for (std::shared_ptr<Event> event : _income_events) {
			event->resolve();
		}

		//ñîğòèğîâêà ñïèñêà ïî âîçğàñòàíèş
		std::vector<std::string> names;
		for (std::shared_ptr<Client> client : _clients) {
			names.push_back(client->getName());
		}
		std::sort(names.begin(), names.end());
		for (std::string name : names) {
			for (std::shared_ptr<Client> client : _clients) {
				if (name == client->getName()) {
					this->output.push_back(_time_end_string + ' ' + std::to_string(11) + ' ' + client->getName());
					this->clientLeave(client->getName(), _time_end_string);
					break;
				}
			}
		}
		this->output.push_back(_time_end_string);
		for (std::shared_ptr<Computer> computer : _computers) {
			this->output.push_back(std::to_string(computer->getId()) + ' ' + std::to_string(computer->getCashAmount()) + ' ' + computer->getAllWorkTime());
		}
	}
	
	void setHourPrice(int hour_price) {
		_hour_price = hour_price;
	}
	void setComputersAmount(int computers_amount) {
		_computers_amount = computers_amount;
		_computers.clear();
		_computers.reserve(computers_amount);
		for (int i = 1; i <= computers_amount; ++i) {
			_computers.push_back(std::make_shared<Computer>(*this, i));
		}
	}
	int getComputersAmount() {
		return _computers_amount;
	}
	void setTimeBegin(std::string& time) {
		_time_begin_string = time;

	}
	void setTimeEnd(std::string& time) {
		_time_end_string = time;
	}
	std::string getTimeBegin() {
		return _time_begin_string;
	}
	std::string getTimeEnd() {
		return _time_end_string;
	}
	void addIncomeEvent(std::shared_ptr<Event> event) {//ÌÎÃÓÒ ÁÛÒÜ ÏĞÎÁËÅÌÛ Ñ ÒÅÌ ×ÒÎ ÅÂÅÍÒ İÒÎ ÊËŞ×ÅÂÎÅ ÑËÎÂÎ, ÍÀÄÅÆÄÀ ÍÀ ÊÎÌÏÈËßÒÎĞ. ÏĞÈ ÍÅÎÁÕÎÄÈÌÎÑÒÈ ÈÇÌÅÍÈÒÜ ËÎÊÀËÜÍÎÅ ÈÌß
		_income_events.push_back(event);
	}
	size_t queueSize() {
		return _queue.size();
	}
	void addInQueue(std::string& name) {
		_queue.push(this->getClientByName(name));
	}
	std::shared_ptr<Client> getFromQueue() {
		std::shared_ptr<Client> temp = this->_queue.front();
		this->_queue.pop();
		return temp;
	}
	std::vector<std::string> output{};
private:
	int _computers_amount;
	std::vector<std::shared_ptr<Computer>> _computers;
	std::list<std::shared_ptr<Client>> _clients;
	std::string _time_begin_string;
	std::string _time_end_string;
	int _hour_price;
	std::vector<std::shared_ptr<Event>> _income_events;
	std::queue<std::shared_ptr<Client>> _queue;
};
int get_Minutes(std::string& timestamp) {
	return std::stoi(timestamp.substr(timestamp.find(':') + 1));
}
int get_Hours(std::string& timestamp) {

	return std::stoi(timestamp.substr(0, timestamp.find(':')));
}
void validate(std::vector<std::string>& input, Computer_Club& YADRO) {

	std::vector<std::string>::iterator it = input.begin();
	std::smatch smatches{};

	if (!std::regex_match(*it, std::regex{ "\\d+", std::regex::ECMAScript })) throw* it;
	else YADRO.setComputersAmount(std::stoi(*it));
	++it;
	
	if ((!std::regex_match(*it, smatches, std::regex{ "((?:[01][0-9]|2[0-3]):[0-5]\\d)\\s((?:[01][0-9]|2[0-3]):[0-5]\\d)", std::regex::ECMAScript })) || smatches[1] > smatches[2]) throw* it;
	else {
		std::string time_begin{ smatches[1] };
		std::string time_end{ smatches[2] };
		YADRO.setTimeBegin(time_begin);
		YADRO.setTimeEnd(time_end);

		YADRO.output.push_back(YADRO.getTimeBegin());
	}
	++it;
	
	if (!std::regex_match(*it, std::regex{ "[0-9]+", std::regex::ECMAScript })) throw* it;
	else YADRO.setHourPrice(std::stoi(*it));
	++it;
	
	std::regex_match(*it, smatches, std::regex{ "((?:[01][0-9]|2[0-3]):[0-5]\\d)\\s([1-4])\\s([a-z0-9_\\-]+)(?:\\s(\\d+))?" });
	std::string prev_time{ smatches[1] };
	//ïåğåáîğ âõîäÿùèõ ñîáûòèé
	for (; it != input.end(); ++it) {
		if (!std::regex_match(*it, smatches, std::regex{ "((?:[01][0-9]|2[0-3]):[0-5]\\d)\\s([1-4])\\s([a-z0-9_\\-]+)(?:\\s(\\d+))?" })) throw* it;
		else {
			std::string time{ smatches[1] };
			if (time < prev_time) throw* it;
			else prev_time = time;
			int event_id{ std::stoi(smatches[2]) };
			std::string client{ smatches[3] };
			if (event_id == 2) {
				int target_comp{ std::stoi(smatches[4]) };
				YADRO.addIncomeEvent(std::make_shared<Computer_Club::Event>(YADRO, time, event_id, client, target_comp));
			}
			else {
				YADRO.addIncomeEvent(std::make_shared<Computer_Club::Event>(YADRO, time, event_id, client));
			}
		}
	}
}

int main(int argc, const char* argv[]) {
	if (argc != 2) return 1;
	std::ifstream test_file{ argv[1] };
	std::vector<std::string> input{};
	if (test_file.is_open())
	{
		std::string temp_string{};
		while (std::getline(test_file, temp_string)) {
			input.push_back(temp_string);
		}
	}
	else {
		//std::cout << "Error opening file"; //ÏÎ ÒÇ ÍÅ ÓÊÀÇÀÍÎ ÂÛÂÎÄÈÒÜ ÒÀÊÈÅ ÑÎÎÁÙÅÍÈß ÎÁ ÎØÈÁÊÅ
		return 1;
	}
	test_file.close();
	Computer_Club YADRO;
	try {
		validate(input, YADRO);
	}
	catch (std::string& wrong_string) {
		std::cout << wrong_string;
		return 1;
	}
	try {
		YADRO.run();
	}
	catch (int) {
		return 1;
	}
	for (std::string& line : YADRO.output) {
		std::cout << line << '\n';
	}
	return 0;
}