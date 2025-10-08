#include <cstdio>
#include <cstring>
#include <cctype>

#include <chrono>
#include <string>
#include <iostream>

// aduh gantengnya
const int ASCII_LOWER_START_INDEX = 97;
const int ASCII_UPPER_START_INDEX = 65;
const int ALPHABET_LEN = 26;
const int MATRIX_SIZE = 5;
const bool IGNORE_SYMBOL = true;

typedef enum Method {
	METHOD_ENCRYPT = 0,
	METHOD_DECRYPT
} Method;

typedef struct MatrixPosition {
	int at_row;
	int at_column;
} MatrixPosition;

typedef struct Pair {
	int left;
	int right;
} Pair;

int usage(const char* filename) {
    std::printf("\n=== [ Panduan Penggunaan Program Enkripsi ] ===\n");
    std::printf("Perhatikan ya, Azril\n\n");

    std::printf("Cara pakai:\n");
    std::printf("   %s <method> <cipher-mode>\n\n", filename);

    std::printf("Parameter:\n");
    std::printf("   <method>       : encrypt | decrypt\n");
    std::printf("   <cipher-mode>  : caesar | playfair | compare-speed\n\n");

    std::printf("Contoh penggunaan:\n");
    std::printf("   %s encrypt caesar\n", filename);
    std::printf("   %s decrypt playfair\n", filename);
    std::printf("   %s encrypt compare-speed\n\n", filename);
    std::printf("   %s decrypt compare-speed\n\n", filename);

    std::printf("Catatan:\n");
    std::printf(" - Saat ini hanya huruf (A-Z) yang dapat dienkripsi.\n");
    std::printf(" - Angka, spasi, dan karakter unik tidak akan dienkripsi.\n");
    std::printf(" - Untuk Playfair, huruf 'J' akan digabung dengan 'I'.\n");
    std::printf(" - Gunakan 'compare-speed' untuk mengukur kecepatan kedua cipher.\n");

    std::printf("===============================================\n\n");
    return 1;
}

void caesar(std::string content, std::string& result, Method method) {
	int shift = 0;

	std::printf("Shift Karakter (X): ");
	std::cin >> shift;

	int indexing_mode = ASCII_LOWER_START_INDEX;
	for (char lachar : content) {
		char result_char = lachar;

		if(!std::isalpha(result_char)) { 
			result.push_back(result_char);
			continue;
		}

		if(((int)lachar) >= ASCII_LOWER_START_INDEX) {
			indexing_mode = ASCII_LOWER_START_INDEX;
		}
		else indexing_mode = ASCII_UPPER_START_INDEX;

		int move_start_index = ((int) lachar) - indexing_mode;
		int shifting = 0;

		if(method == METHOD_ENCRYPT) {
			shifting = (move_start_index + shift) % ALPHABET_LEN;
		} else if (method == METHOD_DECRYPT) {
			shifting = ((move_start_index - shift) % ALPHABET_LEN + ALPHABET_LEN) % ALPHABET_LEN;
		}

		int reset_index = shifting + indexing_mode;
		result_char = (char)reset_index;
		result.push_back(result_char);
	}
}


MatrixPosition get_matrix_position(int position) {
    return { position / MATRIX_SIZE, position % MATRIX_SIZE };
}

int get_index_position_int(int row, int col) {
    return row * MATRIX_SIZE + col;
}

int move_previous_in_row(int current_idx) {
    MatrixPosition coords = get_matrix_position(current_idx);
    int new_col = (coords.at_column - 1 + MATRIX_SIZE) % MATRIX_SIZE;
    return get_index_position_int(coords.at_row, new_col);
}

int move_previous_in_column(int current_idx) {
    MatrixPosition coords = get_matrix_position(current_idx);
    int new_row = (coords.at_row - 1 + MATRIX_SIZE) % MATRIX_SIZE;
    return get_index_position_int(new_row, coords.at_column);
}

int move_next_in_row(int current_idx) {
    MatrixPosition coords = get_matrix_position(current_idx);
    int new_col = (coords.at_column + 1) % MATRIX_SIZE;
    return get_index_position_int(coords.at_row, new_col);
}

int move_next_in_column(int current_idx) {
    MatrixPosition coords = get_matrix_position(current_idx);
    int new_row = (coords.at_row + 1) % MATRIX_SIZE;
    return get_index_position_int(new_row, coords.at_column);
}

bool same_row(int idx1, int idx2) { return idx1 / MATRIX_SIZE == idx2 / MATRIX_SIZE; }
bool same_col(int idx1, int idx2) { return idx1 % MATRIX_SIZE == idx2 % MATRIX_SIZE; }

// ---------------------- Swap Logic ----------------------

Pair swap_char(char left, char right, const std::string& key, Method method) {
    int idx_left = key.find(left);
    int idx_right = key.find(right);

    int next_left = 0;
    int next_right = 0;

    if (same_row(idx_left, idx_right)) {
        if (method == METHOD_ENCRYPT) {
            next_left = move_next_in_row(idx_left);
            next_right = move_next_in_row(idx_right);
        } else {
            next_left = move_previous_in_row(idx_left);
            next_right = move_previous_in_row(idx_right);
        }
    } else if (same_col(idx_left, idx_right)) {
        if (method == METHOD_ENCRYPT) {
            next_left = move_next_in_column(idx_left);
            next_right = move_next_in_column(idx_right);
        } else {
            next_left = move_previous_in_column(idx_left);
            next_right = move_previous_in_column(idx_right);
        }
    } else { // rectangle swap
        MatrixPosition left_pos = get_matrix_position(idx_left);
        MatrixPosition right_pos = get_matrix_position(idx_right);
        next_left = get_index_position_int(left_pos.at_row, right_pos.at_column);
        next_right = get_index_position_int(right_pos.at_row, left_pos.at_column);
    }

    return { key[next_left], key[next_right] };
}

// ---------------------- Key Preparation ----------------------

std::string prepare_key(const std::string& raw_key) {
    std::string clean_key;
    clean_key.reserve(25);

    for (char c : raw_key) {
        if (c == ' ' || c == 'J' || c == 'j') continue;
        c = std::toupper(c);
        if (clean_key.find(c) == std::string::npos)
            clean_key.push_back(c);
    }

    for (int i = 0; i < ALPHABET_LEN; ++i) {
        char c = (char)(ASCII_UPPER_START_INDEX + i);
        if (c == 'J') continue;
        if (clean_key.find(c) == std::string::npos)
            clean_key.push_back(c);
    }

    return clean_key;
}

// ---------------------- Playfair ----------------------

void playfair(const std::string& ori_content, std::string& result, Method method) {
	std::string raw_key;

	std::printf("Masukan Key: ");
	std::cin >> raw_key;

    for (char c : raw_key) {
		if(!std::isalpha(c)) {
			std::printf("[KESALAHAN]: Karakter tidak valid '%c' terdeteksi. Playfair hanya mendukung huruf A-Z.\n", c);
        	throw std::runtime_error("Playfair: ditemukan simbol non-alfabet"); 
		}
	}

	std::string key = prepare_key(raw_key);

    std::string content;
    for (char c : ori_content) {
        if (c == ' ' || c == 'J' || c == 'j') continue;
        content.push_back(std::toupper(c));
    }

    size_t i = 0;
    while (i < content.length()) {
		if (IGNORE_SYMBOL && !std::isalpha(content[i])) {
			result.push_back(content[i]);
			i++;
			continue;
		 } else if (!std::isalpha(content[i])) {
			std::printf("[KESALAHAN]: Karakter tidak valid '%c' terdeteksi. Playfair hanya mendukung huruf A-Z.\n", content[i]);
        	throw std::runtime_error("Playfair: ditemukan simbol non-alfabet");	
		 }

        char left_char = content[i];
        char right_char;

        if (i + 1 >= content.length() || content[i] == content[i + 1])
            right_char = 'X';
        else
            right_char = content[i + 1];

        Pair pair = swap_char(left_char, right_char, key, method);
        result.push_back(pair.left);
        result.push_back(pair.right);

        i += (right_char == 'X' && left_char == content[i]) ? 1 : 2;
    }
}

void compare(const std::string& ori_content, Method method) {
	using namespace std::chrono;

    std::string caesar_result;
    std::string playfair_result;

	// measure caesar
    std::chrono::high_resolution_clock::time_point start_c;
    std::chrono::high_resolution_clock::time_point end_c;
    long long caesar_time_us = 0;

	start_c = std::chrono::high_resolution_clock::now();
	caesar(ori_content, caesar_result, method);
	end_c = std::chrono::high_resolution_clock::now();
    caesar_time_us = std::chrono::duration_cast<std::chrono::microseconds>(end_c - start_c).count();

	// measure playfair 
	std::chrono::high_resolution_clock::time_point start_p;
    std::chrono::high_resolution_clock::time_point end_p;
    long long playfair_time_us = 0;

    start_p = std::chrono::high_resolution_clock::now();

	try {
		playfair(ori_content, playfair_result, method);
	} catch (const std::exception& e) {
		std::printf("Enkripsi dibatalkan: %s\n", e.what());
		return;
	}

	end_p = std::chrono::high_resolution_clock::now();
    playfair_time_us = std::chrono::duration_cast<std::chrono::microseconds>(end_p - start_p).count();

	std::printf("Hasil Cipher: %s\n", caesar_result.c_str());
	std::printf("Hasil PLayfair: %s\n", playfair_result.c_str());
	

	std::printf("\n[PERBEDAAN KECEPARAN]\n");
    std::printf("Caesar Cipher   : 0.%lld seconds\n", caesar_time_us);
    std::printf("Playfair Cipher : 0.%lld seconds\n", playfair_time_us);

    if (caesar_time_us < playfair_time_us)
		std::printf("=> Caesar lebih cepat 0.%lld microseconds\n", playfair_time_us - caesar_time_us);
    else if (playfair_time_us < caesar_time_us)
		std::printf("=> Playfair lebih cepat 0.%lld microseconds\n", caesar_time_us - playfair_time_us);
    else
		std::printf("Kecepatannya sama\n");
}

int main(int argc, char* argv[]) {
	if (argc <= 2) return usage(argv[0]);

	char* method = argv[1];
	char* chiper = argv[2];

	// azril cek dulu
	if(
		std::strcmp(chiper, "caesar") != 0 && 
		std::strcmp(chiper, "playfair") != 0 && 
		std::strcmp(chiper, "compare-speed") != 0
	) return usage(argv[0]);

	std::string content;
	std::string result = "";

	if(std::strcmp(method, "encrypt") == 0) {
		std::printf("Masukan Text: ");
		std::getline(std::cin, content);
		result.reserve(content.length());

		for (char c : content) {
			if(!std::isalpha(c)) {
				std::printf("[KESALAHAN]: Karakter tidak valid '%c' terdeteksi. Playfair hanya mendukung huruf A-Z.\n", c);
				return 1;
			}
		}


		if(std::strcmp(chiper, "caesar") == 0) caesar(content, result, METHOD_ENCRYPT);
		else if(std::strcmp(chiper, "playfair") == 0) {
			try {
				playfair(content, result, METHOD_ENCRYPT);
			} catch (const std::exception& e) {
				std::printf("Enkripsi dibatalkan: %s\n", e.what());
				return 1;
			}
		} else if(std::strcmp(chiper, "compare-speed") == 0) compare(content, METHOD_ENCRYPT);
		else return usage(argv[1]);
	} else if (std::strcmp(method, "decrypt") == 0) {
		std::printf("Masukan Text: ");
		std::getline(std::cin, content);
		result.reserve(content.length());

		for (char c : content) {
			if(!std::isalpha(c)) {
				std::printf("[KESALAHAN]: Karakter tidak valid '%c' terdeteksi. Playfair hanya mendukung huruf A-Z.\n", c);
				return 1;
			}
		}

		if(std::strcmp(chiper, "caesar") == 0) caesar(content, result, METHOD_DECRYPT);
		else if(std::strcmp(chiper, "playfair") == 0) { 
			try {
				playfair(content, result, METHOD_DECRYPT);
			} catch (const std::exception& e) {
				std::printf("Enkripsi dibatalkan: %s\n", e.what());
				return 1;
			}
		} else if(std::strcmp(chiper, "compare-speed") == 0) compare(content, METHOD_DECRYPT);
		else return usage(argv[1]);
	} else {
		return usage(argv[1]);
	}

	if(std::strcmp(chiper, "compare-speed") != 0)
		std::printf("Hasil: %s\n", result.c_str());

	return 0;
}
